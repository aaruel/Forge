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

// System Headers
#include <PolyVox/CubicSurfaceExtractor.h>
#include <PolyVox/MarchingCubesSurfaceExtractor.h>
#include <PolyVox/Mesh.h>
#include <PolyVox/RawVolume.h>
#include <PolyVox/MaterialDensityPair.h>
#include <PolyVox/Raycast.h>
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <future>
#include <chrono>

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

    class Voxel {
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
        
    public:
        Voxel(Shader * shader);
        ~Voxel();
        
        void render();
        bool getVoxelFromEye(PolyVox::Vector3DInt32 & result, bool adjacent = true);
        
    private:
    
        PolyVox::RawVolume<PolyVox::MaterialDensityPair88>* volumes;
        OpenGLMeshData mData;
        Shader * mShader;
        Camera * mCamera;
        
        /// Multithreading volume to mesh decoding
        bool expectMesher = false;
        std::future<void> mesherFuture;
        std::function<void()> * mesherPayload;
        bool mesherReady() {
            return expectMesher
                && mesherFuture.wait_for(std::chrono::seconds(0)) == std::future_status::ready;
        }
        
    };
};

#endif /* voxel_h */
