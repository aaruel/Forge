//
//  voxel.cpp
//  Glitter
//
//  Created by Aaron Ruel on 4/1/19.
//

#include "voxel.hpp"

using namespace PolyVox;

namespace XK {
    //////////////
    /// Public ///
    //////////////

    Voxel::Voxel(Shader * shader) : mShader(shader), mCamera(Camera::getInstance()) {
        // Generate volumetric mesh
        auto volData = new RawVolume<MaterialDensityPair88>(Region(
            Vector3DInt32(0, 0, 0),
            Vector3DInt32(63, 63, 63)
        ));
        createSphereInVolume(*volData, 30, 1);
        volumes = (volData);
        
        makeRenderable();
    }
    
    void Voxel::render() {
        // Our example framework only uses a single shader for the scene (for all meshes).
        mShader->activate();

        // These two matrices are constant for all meshes.
        mCamera->render(mShader->get());

        // Iterate over each mesh which the user added to our list, and render it.
        {
            OpenGLMeshData &meshData = mData;
            //Set up the model matrrix based on provided translation and scale.
            glm::mat4 modelMatrix = glm::mat4(1.0f);
            glm::translate(modelMatrix, meshData.translation);
            glm::scale(modelMatrix, glm::vec3(meshData.scale));
            GLint Umodel = glGetUniformLocation(mShader->get(), "model");
            glUniformMatrix4fv(Umodel, 1, GL_FALSE, glm::value_ptr(modelMatrix));

            // Bind the vertex array for the current mesh
            glBindVertexArray(meshData.vertexArrayObject);
            // Draw the mesh
            glDrawElements(GL_TRIANGLES, meshData.noOfIndices, meshData.indexType, 0);
            // Unbind the vertex array.
            glBindVertexArray(0);
        }
        
        // Try to raycast for block updates
        if (Input::getInstance()->isLeftClick()) {
            Vector3DInt32 newVox;
            if (!getVoxelFromEye(newVox)) return;
            addVoxel(newVox);
        }
    }
    
    void Voxel::addVoxel(Vector3DInt32 position) {
        // Clean up previous GPU array data
        glDeleteVertexArrays(1, &mData.vertexArrayObject);
        
        // Modify volume
        volumes->setVoxel(
            position.getX(),
            position.getY(),
            position.getZ(),
            MaterialDensityPair88(1, 1)
        );
        
        makeRenderable();
    }
    
    ////////////////////////
    /// EXTERNAL FUNCTOR ///
    ////////////////////////
    class RaycastTestFunctor {
    public:
        RaycastTestFunctor()
            : mValid(false)
        {}

        bool operator()(const RawVolume<MaterialDensityPair88>::Sampler& sampler) {
            // Run until the ray hits a voxel with volume
            MaterialDensityPair88 current = sampler.getVoxel();
            if (sampler.isCurrentPositionValid() && current != mInvalidVoxel) {
                mValid = true;
                // Stop iterating after finding valid voxel
                return false;
            }
            
            // Save the previous block! Nice trick to write an adjacent block
            location = sampler.getPosition();
            
            // We are in the volume, so decide whether to continue based on the voxel value.
            return true;
        }
        
        MaterialDensityPair88 mInvalidVoxel = MaterialDensityPair88(0, 0);
        bool mValid;
        Vector3DInt32 location;
    };
    
    bool Voxel::getVoxelFromEye(Vector3DInt32 & result) {
        glm::vec3 position = mCamera->getPosition();
        // The eye is normalized, extended by 100
        glm::vec3 eye = mCamera->getEye() * 100.0f;
        Vector3DFloat vp = Vector3DFloat(position.x, position.y, position.z);
        Vector3DFloat ve = Vector3DFloat(eye.x, eye.y, eye.z);
        RaycastTestFunctor raycastCallback;
        
        // Raycast
        raycastWithDirection(volumes, vp, ve, raycastCallback);
        
        result = Vector3DInt32(raycastCallback.location);
        
        // Get block to modify
        return raycastCallback.mValid;
    }
    
    Voxel::~Voxel() {
        delete volumes;
    }
    
    ///////////////
    /// Private ///
    ///////////////
    
    void Voxel::makeRenderable() {
        // Generate renderable
        auto mesh = extractCubicMesh(volumes, volumes->getEnclosingRegion());
        auto decoMesh = decodeMesh(mesh);
        addMesh(decoMesh);
    }
    
    void Voxel::createSphereInVolume(RawVolume<MaterialDensityPair88>& volData, float fRadius, uint8_t uValue) {
        //This vector hold the position of the center of the volume
        Vector3DInt32 v3dVolCenter = (volData.getEnclosingRegion().getUpperCorner() - volData.getEnclosingRegion().getLowerCorner()) / static_cast<int32_t>(2);

        //This three-level for loop iterates over every voxel in the volume
        for (int z = 0; z < volData.getDepth(); z++)
        for (int y = 0; y < volData.getHeight(); y++)
        for (int x = 0; x < volData.getWidth(); x++) {
            //Store our current position as a vector...
            Vector3DInt32 v3dCurrentPos(x, y, z);
            //And compute how far the current position is from the center of the volume
            double fDistToCenter = (v3dCurrentPos - v3dVolCenter).length();

            //If the current voxel is less than 'radius' units from the center
            //then we make it solid, otherwise we make it empty space.
            if (fDistToCenter <= fRadius) {
            
                unsigned short d = uValue > 0
                    ? MaterialDensityPair88::getMaxDensity()
                    : MaterialDensityPair88::getMinDensity();
                
                volData.setVoxel(
                    x,
                    y,
                    z,
                    MaterialDensityPair88(uValue, d)
                );
                
            }
        }
    }
    
    template<typename MeshType>
    void Voxel::addMesh(
        const MeshType& surfaceMesh,
        const PolyVox::Vector3DInt32& translation,
        float scale
    ) {
        // This struct holds the OpenGL properties (buffer handles, etc) which will be used
        // to render our mesh. We copy the data from the PolyVox mesh into this structure.
        OpenGLMeshData meshData;

        // Create the VAO for the mesh
        glGenVertexArrays(1, &(meshData.vertexArrayObject));
        glBindVertexArray(meshData.vertexArrayObject);

        // The GL_ARRAY_BUFFER will contain the list of vertex positions
        glGenBuffers(1, &(meshData.vertexBuffer));
        glBindBuffer(GL_ARRAY_BUFFER, meshData.vertexBuffer);
        glBufferData(GL_ARRAY_BUFFER, surfaceMesh.getNoOfVertices() * sizeof(typename MeshType::VertexType), surfaceMesh.getRawVertexData(), GL_STATIC_DRAW);

        // and GL_ELEMENT_ARRAY_BUFFER will contain the indices
        glGenBuffers(1, &(meshData.indexBuffer));
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, meshData.indexBuffer);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, surfaceMesh.getNoOfIndices() * sizeof(typename MeshType::IndexType), surfaceMesh.getRawIndexData(), GL_STATIC_DRAW);

        // Every surface extractor outputs valid positions for the vertices, so tell OpenGL how these are laid out
        glEnableVertexAttribArray(0); // Attrib '0' is the vertex positions
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(typename MeshType::VertexType), (GLvoid*)(offsetof(typename MeshType::VertexType, position))); //take the first 3 floats from every sizeof(decltype(vecVertices)::value_type)

        // Some surface extractors also generate normals, so tell OpenGL how these are laid out. If a surface extractor
        // does not generate normals then nonsense values are written into the buffer here and sghould be ignored by the
        // shader. This is mostly just to simplify this example code - in a real application you will know whether your
        // chosen surface extractor generates normals and can skip uploading them if not.
        glEnableVertexAttribArray(1); // Attrib '1' is the vertex normals.
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(typename MeshType::VertexType), (GLvoid*)(offsetof(typename MeshType::VertexType, normal)));

        // Finally a surface extractor will probably output additional data. This is highly application dependant. For this example code
        // we're just uploading it as a set of bytes which we can read individually, but real code will want to do something specialised here.
        glEnableVertexAttribArray(2); //We're talking about shader attribute '2'
        GLint size = (std::min)(sizeof(typename MeshType::VertexType::DataType), size_t(4)); // Can't upload more that 4 components (vec4 is GLSL's biggest type)
        glVertexAttribIPointer(2, size, GL_UNSIGNED_BYTE, sizeof(typename MeshType::VertexType), (GLvoid*)(offsetof(typename MeshType::VertexType, data)));

        // We're done uploading and can now unbind and delete the buffers.
        // As long as VBOs are referenced in the VAO, nothing bad happens.
        glBindVertexArray(0);
        glDeleteBuffers(1, &meshData.vertexBuffer);
        glDeleteBuffers(1, &meshData.indexBuffer);

        // A few additional properties can be copied across for use during rendering.
        meshData.noOfIndices = surfaceMesh.getNoOfIndices();
        meshData.translation = glm::vec3(translation.getX(), translation.getY(), translation.getZ());
        meshData.scale = scale;

        // Set 16 or 32-bit index buffer size.
        meshData.indexType = sizeof(typename MeshType::IndexType) == 2 ? GL_UNSIGNED_SHORT : GL_UNSIGNED_INT;

        // Now add the mesh to the list of meshes to render.
        mData = (meshData);
    }
}
