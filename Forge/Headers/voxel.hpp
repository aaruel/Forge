//
//  voxel.hpp
//  Glitter
//
//  Created by Aaron Ruel on 4/1/19.
//

#ifndef voxel_h
#define voxel_h

#include "shader.hpp"
#include "camera.hpp"
#include "renderable.hpp"
#include "skybox.hpp"

// System Headers
#include <PolyVox/CubicSurfaceExtractor.h>
#include <PolyVox/MarchingCubesSurfaceExtractor.h>
#include <PolyVox/Mesh.h>
#include <PolyVox/RawVolume.h>
#include <PolyVox/PagedVolume.h>
#include <PolyVox/MaterialDensityPair.h>
#include <PolyVox/Raycast.h>
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <future>
#include <chrono>
#include <vector>

namespace XK {
    struct OpenGLMeshData {
        GLuint noOfIndices;
        GLuint vBufferSize;
        GLuint iBufferSize;
        GLenum indexType;
        GLuint indexBuffer;
        GLuint vertexBuffer;
        GLuint vertexArrayObject;
        glm::vec3 translation;
        float scale;
    };
    
    struct Texture {
        GLuint buffer;
        GLint unit;
        std::string location;
    };

    class Voxel : public Renderable {
    private:
        void createSphereInVolume(PolyVox::RawVolume<PolyVox::MaterialDensityPair88>& volData, float fRadius, uint8_t uValue);
        
        // Constructs a new set of VAO and VBOs
        template<typename MeshType>
        void addMesh(
            const MeshType& surfaceMesh,
            const PolyVox::Vector3DInt32& translation = PolyVox::Vector3DInt32(0, 0, 0),
            float scale = 1.0f
        );
        
        // Uploads new mesh to existing VBOs
        template<typename MeshType>
        void updateMesh(const MeshType& surfaceMesh);
        
        // Set or update existing loaded volume
        void makeRenderable(bool updating = false);
        
        // Add or subtract a voxel to be displayed in the mesh
        void addVoxel(PolyVox::Vector3DInt32 position);
        void deleteVoxel(PolyVox::Vector3DInt32 position);
        
        // Indicates when the mesher thread is ready to be uploaded
        bool mesherReady();
        
    public:
        Voxel(kgr::container * container, Shader * shader);
        ~Voxel();
        
        virtual void render();
        bool getVoxelFromEye(PolyVox::Vector3DInt32 & result, bool adjacent = true);
        // Builder pattern texture adding
        Voxel & addTexture(std::string texLocation, std::string filename);
        
    private:
    
        PolyVox::PagedVolume<PolyVox::MaterialDensityPair88> * volumes;
        OpenGLMeshData mData;
        
        /// Multithreading volume to mesh decoding
        bool expectMesher = false;
        std::future<void> mesherFuture;
        std::function<void()> * mesherPayload;
        
        /// Textures
        int nTextures = 0;
        std::vector<Texture> textures;
        
        // Disable Copying and Assignment
        Voxel(Voxel const &) = delete;
        Voxel & operator=(Voxel const &) = delete;

        // Reflection component
        Skybox * mSkybox = nullptr;
        
    };
};

#endif /* voxel_h */
