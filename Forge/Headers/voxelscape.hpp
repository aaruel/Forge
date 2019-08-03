//
//  voxelscape.hpp
//  Forge
//
//  Created by Aaron Ruel on 5/28/19.
//

#ifndef voxelscape_h
#define voxelscape_h

#include "renderable.hpp"
#include "camera.hpp"
#include "shader.hpp"
#include "octree.hpp"
#include "utils.hpp"
#include "xk.hpp"

#include <Voxels.h>
#include <unordered_map>
#include <array>
#include <glm/glm.hpp>
#include <glm/gtx/string_cast.hpp>
#include <iostream>
#include <functional>

namespace XK {
    class VoxelMaterial : public Voxels::MaterialMap {
        virtual Material* GetMaterial(unsigned char) const override {
            return nullptr;
        }
    };

    template<
        class T,
        std::enable_if_t<std::is_base_of<Voxels::VoxelSurface, T>::value>* = nullptr
    >
    class Voxelscape : public Renderable {
    private:
        struct vRenderParams : RenderParams<BasicVertex> {
            glm::vec3 minCorner;
            glm::vec3 maxCorner;
        };
        using otNode = typename Octree<vRenderParams>::Node;

        // Template Vector Conversion Helpers
        template<typename T1>
        glm::vec3 toVec3(T1 v) { return glm::vec3(v.x, v.y, v.z); }
        template<typename T1>
        glm::vec3 midpoint(T1 v1, T1 v2) { return (toVec3(v1) + toVec3(v2)) / 2.f; }
        template<typename T1>
        glm::vec3 swapYZ3(T1 i) { return glm::vec3(i.x, i.z, i.y); }
        template<typename T1>
        glm::vec4 swapYZ4(T1 i) { return glm::vec4(i.x, i.z, i.y, i.w); }

        bool betweenCuboid(glm::vec3 min, glm::vec3 max, glm::vec3 point) {
            auto past = glm::greaterThanEqual(point, min);
            auto before = glm::lessThan(point, max);
            if (glm::all(past) && glm::all(before)) return true;
            else return false;
        }

        void calculateFrustumPlanes(glm::vec4 m_planes[6]) {
            glm::mat4 m = mCamera->getProjection() * mCamera->getView();

            // left
            m_planes[0].x = m[3][0] + m[0][0];
            m_planes[0].y = m[3][1] + m[0][1];
            m_planes[0].z = m[3][2] + m[0][2];
            m_planes[0].w = m[3][3] + m[0][3];
            // right
            m_planes[1].x = m[3][0] - m[0][0];
            m_planes[1].y = m[3][1] - m[0][1];
            m_planes[1].z = m[3][2] - m[0][2];
            m_planes[1].w = m[3][3] - m[0][3];
            // top
            m_planes[2].x = m[3][0] - m[1][0];
            m_planes[2].y = m[3][1] - m[1][1];
            m_planes[2].z = m[3][2] - m[1][2];
            m_planes[2].w = m[3][3] - m[1][3];
            // bottom
            m_planes[3].x = m[3][0] + m[1][0];
            m_planes[3].y = m[3][1] + m[1][1];
            m_planes[3].z = m[3][2] + m[1][2];
            m_planes[3].w = m[3][3] + m[1][3];
            // near
            m_planes[4].x = m[2][0];
            m_planes[4].y = m[2][1];
            m_planes[4].z = m[2][2];
            m_planes[4].w = m[2][3];
            // far
            m_planes[5].x = m[3][0] - m[2][0];
            m_planes[5].y = m[3][1] - m[2][1];
            m_planes[5].z = m[3][2] - m[2][2];
            m_planes[5].w = m[3][3] - m[2][3];

            for (unsigned i = 0; i < 6; ++i) {
                float norm = sqrt(
                    m_planes[i].x * m_planes[i].x
                    + m_planes[i].y * m_planes[i].y
                    + m_planes[i].z * m_planes[i].z
                );

                m_planes[i].x /= norm;
                m_planes[i].y /= norm;
                m_planes[i].z /= norm;
                m_planes[i].w /= norm;
            }
        }

        void octreePass() {
            glm::vec3 camPos = swapYZ3(mCamera->getPosition());
            glm::vec4 planes[6];
            calculateFrustumPlanes(planes);

            const glm::mat4 invse = glm::inverse(mModel);
            const glm::mat4 xpose = glm::transpose(mModel);

            for (int i = 0; i < 6; ++i) {
                planes[i] = glm::normalize(xpose * planes[i]);
            }

            // inverse transform
            camPos = glm::mat3(invse) * camPos;
        }

        const Voxels::BlockPolygons * findBlock(unsigned level, std::array<glm::vec3, 2>&& bounds) {
            int block = 0;
            bool exists = true;
            do {
                const Voxels::BlockPolygons * test = mPolySurface->GetBlockForLevel(level, block++);
                exists = test != nullptr;
                if (
                    exists
                    && !mUsedNodes[test]
                    && betweenCuboid(
                        bounds[0],
                        bounds[1],
                        midpoint(test->GetMinimalCorner(), test->GetMaximalCorner())
                    )
                ) {
                    mUsedNodes[test] = true;
                    return test;
                }
            } while (exists);
            return nullptr;
        }

        using Bounds = std::array<glm::vec3, 2>;
        vRenderParams uploadAtBlock(unsigned level, Bounds&& bounds, bool firstBlock) {

            const Voxels::BlockPolygons * inBlock = firstBlock ?
                mPolySurface->GetBlockForLevel(level, 0)
                : findBlock(level, std::forward<Bounds>(bounds));

            // if block doesn't exist, return
            if (!inBlock) return vRenderParams();
            // else, upload to gpu
            unsigned vertices, indices;
            auto verts = inBlock->GetVertices(&vertices);
            auto inds = inBlock->GetIndices(&indices);
            // For each block instance a VAO
            vRenderParams rp;
            for (unsigned v = 0; v < vertices; ++v) {
                BasicVertex bv;
                // z is up in this case
                const glm::vec3 p = glm::vec3(
                    verts[v].Position.x,
                    verts[v].Position.z,
                    verts[v].Position.y
                );
                const glm::vec3 n = glm::vec3(
                    verts[v].Normal.x,
                    verts[v].Normal.z,
                    verts[v].Normal.y
                );
                std::get<position>(bv) = p;
                std::get<normal>(bv) = n;
                rp.vertices.push_back(bv);
            }
            for (unsigned i = 0; i < indices; ++i) {
                rp.indices.push_back(inds[i]);
            }
            // Create a buffer for each block
            uploadToGPU(&rp);
            rp.minCorner = swapYZ3(inBlock->GetMinimalCorner());
            rp.maxCorner = swapYZ3(inBlock->GetMaximalCorner());
            return rp;
        }

        void renderNode(otNode* node) {
            // blank node
            if (!node->mData.vao) return;
            printf("%u ", node->mData.vao);
            glBindVertexArray(node->mData.vao);
            glDrawElements(GL_TRIANGLES, node->mData.indices.size(), GL_UNSIGNED_INT, nullptr);
        }
        
        void renderChildren(otNode* node) {
            for (int i = 0; i < 8; ++i) {
                if (node->hasChild(i)) continue;
                else renderNode(node->traverse(i));
            }
        }
        
        bool cameraBetweenNode(otNode* node) {
            return betweenCuboid(node->mData.minCorner, node->mData.maxCorner, mCamera->getPosition());
        }

        int fugg = 0;
        int fuggctr = 0;
        void renderLevel(otNode* node, int level = 2, int position = 0) {
            if (level == 0) {
                if (position == fugg) renderNode(node);
            }
            else {
                for (int i = 0; i < 8; ++i) {
                    renderLevel(node->traverse(i), level-1, i);
                }
            }
        }

        // Traverse through the octree to find the block the camera is in
        // If camera is between the node, traverse down until a child doesn't exist
        // else render node
        void renderOctree(otNode* node) {
            if (cameraBetweenNode(node) && node->hasChild()) {
                for (int i = 0; i < 8; ++i) {
                    renderOctree(node->traverse(i));
                }
            }
            else {
                renderNode(node);
            }
        }

    public:
        Voxelscape(XKState& xk) : mOct(Octree<vRenderParams>()) {
            if (InitializeVoxels(VOXELS_VERSION, nullptr, nullptr) != Voxels::IE_Ok) {
                throw;
            }
            // rendering parameters
            shader.attach("deferred/voxel.vert").attach("deferred/voxel.frag").link();
            setCamera(xk.mainCamera.get());
            setShader(&shader);
            // Construct the voxel grid
            unsigned gridSize = 64;
            const float start_x = -(gridSize / 8.f);
            const float start_y = -(gridSize / 8.f);
            const float start_z = -(gridSize / 8.f);
            const float step = 0.5;
            mGrid = Voxels::Grid::Create(gridSize, gridSize, gridSize, start_x, start_y, start_z, step, &mSurface);
            mPoly = new Voxels::Polygonizer;
            mMaterial = new VoxelMaterial;
            // Build the polygonal surface
            mPolySurface = mPoly->Execute(*mGrid, mMaterial);

            // Octree sorted blocks by LOD
            unsigned levels = mPolySurface->GetLevelsCount();
            otNode* node = mOct.traverse();

            for (unsigned level = 0; level < levels; ++level) {

                unsigned block = 0;

                // insert for level
                std::function<
                    void(otNode*, unsigned, Bounds&&)
                > orecurse = [&](otNode* n, unsigned l, Bounds&& a){
                    // l = tree level comparator
                    // level = current level
                    if (l <= 0) {
                        // ex. levels = 3
                        // 3 - 1 - 0 = level 2
                        // 3 - 1 - 1 = level 1
                        // 3 - 1 - 2 = level 0
                        bool firstBlock = level == 0;
                        n->insert(uploadAtBlock((levels - 1) - level, std::forward<Bounds>(a), firstBlock));
                    }
                    else {
                        for (size_t i = 0; i < 8; ++i) {
                            orecurse(n->traverse(i), l - 1, {n->mData.minCorner, n->mData.maxCorner});
                        }
                    }
                };
                
                orecurse(node, level, {glm::vec3(), glm::vec3()});
            }
        }
        
        ~Voxelscape() {
            delete mPoly;
            delete mMaterial;
            mPolySurface->Destroy();
            DeinitializeVoxels();
        }

        virtual void render() override {
            mShader->activate();
            mCamera->render(mShader->get());
            // Apply shader uniforms
            shader.bind("model", mModel);
            otNode* rp = mOct.traverse();
            printf("Octree Render: ");
            renderOctree(rp);
            printf("\n");
        }
        
    private:
        // Rendering
        Shader shader;
        // Voxels
        T mSurface;
        Octree<vRenderParams> mOct;
        Voxels::Grid * mGrid;
        Voxels::Polygonizer * mPoly;
        Voxels::PolygonSurface * mPolySurface;
        Voxels::MaterialMap * mMaterial;
        std::unordered_map<const Voxels::BlockPolygons*, bool> mUsedNodes;
    };
    
    class VoxelPlane : public Voxels::VoxelSurface {
    public:
        virtual ~VoxelPlane() override {}
        virtual void GetSurface(
            float xStart, float xEnd, float xStep,
            float yStart, float yEnd, float yStep,
            float zStart, float zEnd, float zStep,
            float* output, unsigned char* materialid, unsigned char* blend
        ) override;
    };
}

#endif /* voxelscape_h */
