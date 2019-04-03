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

namespace XK {
    struct OpenGLMeshData {
        GLuint noOfIndices;
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
        
        template<typename MeshType>
        void addMesh(
            const MeshType& surfaceMesh,
            const PolyVox::Vector3DInt32& translation = PolyVox::Vector3DInt32(0, 0, 0),
            float scale = 1.0f
        );
        void makeRenderable();
        void addVoxel(PolyVox::Vector3DInt32 position);
        
    public:
        Voxel(Shader * shader);
        ~Voxel();
        
        void render();
        bool getVoxelFromEye(PolyVox::Vector3DInt32 & result);
        
    private:
    
        PolyVox::RawVolume<PolyVox::MaterialDensityPair88>* volumes;
        OpenGLMeshData mData;
        Shader * mShader;
        Camera * mCamera;
    };
};

#endif /* voxel_h */
