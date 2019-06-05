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

#include <Voxels.h>
#include <glm/glm.hpp>
#include <iostream>

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
    public:
        Voxelscape() {
            if (InitializeVoxels(VOXELS_VERSION, nullptr, nullptr) != Voxels::IE_Ok) {
                throw;
            }
            // rendering parameters
            shader.attach("deferred/voxel.vert").attach("deferred/voxel.frag").link();
            setCamera(Camera::getInstance());
            setShader(&shader);
            // Construct the voxel grid
            mGrid = Voxels::Grid::Create(64, 64, 64, 0, 0, 0, 1, &mSurface);
            mPoly = new Voxels::Polygonizer;
            mMaterial = new VoxelMaterial;
            // Build the polygonal surface
            mPolySurface = mPoly->Execute(*mGrid, mMaterial);
            // Upload to GPU
            unsigned levels = mPolySurface->GetLevelsCount();
            for (unsigned level = 0; level < levels; ++level) {
                unsigned blocks = mPolySurface->GetBlocksForLevelCount(levels-1);
                for (unsigned block = 0; block < blocks; ++block) {
                    const Voxels::BlockPolygons * inBlock = mPolySurface->GetBlockForLevel(levels-1, block);
                    unsigned vertices, indices;
                    inBlock->GetId();
                    auto verts = inBlock->GetVertices(&vertices);
                    auto inds = inBlock->GetIndices(&indices);
                    for (unsigned v = 0; v < vertices; ++v) {
                        BasicVertex bv;
                        const glm::vec3 p = glm::vec3(
                            verts[v].Position.x,
                            verts[v].Position.y,
                            verts[v].Position.z
                        );
                        const glm::vec3 n = glm::vec3(
                            verts[v].Normal.x,
                            verts[v].Normal.y,
                            verts[v].Normal.z
                        );
                        std::get<position>(bv) = p;
                        std::get<normal>(bv) = n;
                        mRP.vertices.push_back(bv);
                    }
                    for (unsigned i = 0; i < indices; ++i) {
                        mRP.indices.push_back(inds[i]);
                    }
                }
                break;
            }
            uploadToGPU(&mRP);
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
            // Draw vertices
            glBindVertexArray(mVAO);
            // Runs shaders :)
            glDrawElements(GL_TRIANGLES, mRP.indices.size(), GL_UNSIGNED_INT, nullptr);
        }
        
    private:
        // Rendering
        Shader shader;
        // Voxels
        T mSurface;
        Voxels::Grid * mGrid;
        Voxels::Polygonizer * mPoly;
        Voxels::PolygonSurface * mPolySurface;
        Voxels::MaterialMap * mMaterial;
        RenderParams<BasicVertex> mRP;
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
