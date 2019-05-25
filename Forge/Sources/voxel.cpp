//
//  voxel.cpp
//  Glitter
//
//  Created by Aaron Ruel on 4/1/19.
//

#include <stb_image.h>

#include "voxel.hpp"
#include "voxelfunctors.hpp"
#include "utils.hpp"

using namespace PolyVox;

namespace XK {
    //////////////
    /// Public ///
    //////////////

    Voxel::Voxel(Shader * shader) {
        mShader = shader;
        mCamera = Camera::getInstance();
        auto * pager = new FastNoisePager<MaterialDensityPair88>();
        
        // Generate volumetric mesh
        volumes = new PagedVolume<MaterialDensityPair88>(pager, 16 * 1024 * 1024, 64);
        // volumes = new RawVolume<MaterialDensityPair88>(Region(Vector3DInt32(0), Vector3DInt32(64)));
        // createSphereInVolume(*volumes, 30, 1);
        
        makeRenderable();
    }
    
    Voxel::~Voxel() {
        delete volumes;
    }
    
    void Voxel::render() {
        // Switch to the voxel shader
        mShader->activate();

        GLuint nShader = mShader->get();

        // Send view + projection to voxel shader
        mCamera->render(nShader);
        
        //Set up the model matrix based on provided translation and scale.
        glm::mat4 modelMatrix = glm::mat4(1.0f);
        glm::translate(modelMatrix, mData.translation);
        glm::scale(modelMatrix, glm::vec3(mData.scale));
        
        // Send variables
        GLint Umodel = glGetUniformLocation(nShader, "model");
        glUniformMatrix4fv(Umodel, 1, GL_FALSE, glm::value_ptr(modelMatrix));

        // Process added textures
        for (Texture & texture : textures) {
            GLuint textureLocation = glGetUniformLocation(nShader, texture.location.c_str());
            glUniform1i(textureLocation, texture.unit);
            glActiveTexture(GL_TEXTURE0 + texture.unit);
            glBindTexture(GL_TEXTURE_2D, texture.buffer);
        }

        // Bind the vertex array for the current mesh
        glBindVertexArray(mData.vertexArrayObject);
        // Draw the mesh
        glDrawElements(GL_TRIANGLES, mData.noOfIndices, mData.indexType, 0);
        // Unbind the vertex array.
        glBindVertexArray(0);
        
        // POST RENDERING //
        
        // Try to raycast for block updates
        Input * input = Input::getInstance();
        if (input->isLeftClick()) {
            Vector3DInt32 newVox;
            if (!getVoxelFromEye(newVox)) return;
            addVoxel(newVox);
        }
        
        if (input->isRightClick()) {
            Vector3DInt32 newVox;
            if (!getVoxelFromEye(newVox, false)) return;
            deleteVoxel(newVox);
        }
        
        // rendering after a voxel modification thread completes
        if (mesherReady()) {
            (*mesherPayload)();
            delete mesherPayload;
            expectMesher = false;
        }
    }
    
    Voxel & Voxel::addTexture(std::string texLocation, std::string filename) {
        // Define Some Local Variables
        GLenum format;
        // texture
        Texture texture;
        texture.location = texLocation;
        texture.unit = nTextures++;
        std::string mode;
        int width, height, channels;

        // Load the Texture Image from File
        filename = PROJECT_SOURCE_DIR "/Textures/" + filename;
        unsigned char * image = stbi_load(filename.c_str(), & width, & height, & channels, 0);
        if (!image) fprintf(stderr, "%s %s\n", "Failed to Load Texture", filename.c_str());

        // Set the Correct Channel Format
        switch (channels) {
            case 1 : format = GL_ALPHA;     break;
            case 2 : format = GL_LUMINANCE; break;
            case 3 : format = GL_RGB;       break;
            case 4 : format = GL_RGBA;      break;
        }

        // Bind Texture and Set Filtering Levels
        glGenTextures(1, & texture.buffer);
        glBindTexture(GL_TEXTURE_2D, texture.buffer);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexImage2D(
            GL_TEXTURE_2D, 0, format,
            width, height, 0, format, GL_UNSIGNED_BYTE, image
        );
        glGenerateMipmap(GL_TEXTURE_2D);

        // Release Image Pointer and Store the Texture
        stbi_image_free(image);
        
        textures.push_back(texture);
        
        return *this;
    }
    
    void Voxel::addVoxel(Vector3DInt32 position) {
        // Modify volume
        volumes->setVoxel(
            position.getX(),
            position.getY(),
            position.getZ(),
            MaterialDensityPair88(1, MaterialDensityPair88::getMaxDensity()) // note here
        );
        
        makeRenderable(true);
    }
    
    void Voxel::deleteVoxel(Vector3DInt32 position) {
        // Modify volume
        volumes->setVoxel(
            position.getX(),
            position.getY(),
            position.getZ(),
            MaterialDensityPair88(0, MaterialDensityPair88::getMinDensity()) // note here
        );
        
        makeRenderable(true);
    }
    
    bool Voxel::getVoxelFromEye(Vector3DInt32 & result, bool adjacent) {
        glm::vec3 position = mCamera->getPosition();
        // The eye is normalized, extended by 100
        glm::vec3 eye = mCamera->getEye() * 100.0f;
        Vector3DFloat vp = Vector3DFloat(position.x, position.y, position.z);
        Vector3DFloat ve = Vector3DFloat(eye.x, eye.y, eye.z);
        RaycastTest raycastCallback;
        
        // Raycast
        raycastWithDirection(volumes, vp, ve, raycastCallback);
        
        if (adjacent) result = Vector3DInt32(raycastCallback.prevLocation);
        else result = Vector3DInt32(raycastCallback.location);
        // Get block to modify
        return raycastCallback.mValid;
    }
    
    ///////////////
    /// Private ///
    ///////////////
    
    bool Voxel::mesherReady() {
        using secs = std::chrono::seconds;
        using future = std::future_status;
        future status = mesherFuture.wait_for(secs(0));
        return expectMesher && status == future::ready;
    }
    
    void Voxel::makeRenderable(bool updating) {
        expectMesher = true;
        mesherFuture = std::async([this, &updating](){
            // Relatively slow
            // Generate renderable
            Region reg(Vector3DInt32(0, 0, 0), Vector3DInt32(254, 254, 254));
            auto mesh = extractMarchingCubesMesh(volumes, reg);
            // auto mesh = extractMarchingCubesMesh(volumes, volumes->getEnclosingRegion());
            // Decode
            auto decoMesh = decodeMesh(mesh);
            if (updating) {
                mesherPayload = new std::function<void()>(
                    [this, decoMesh](){updateMesh(decoMesh);}
                );
            }
            else {
                mesherPayload = new std::function<void()>(
                    [this, decoMesh](){addMesh(decoMesh);}
                );
            }
        });
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
            uint8_t uVoxelValue = 0;
            float val = fRadius - fDistToCenter; // val is positive when inside sphere
            val = clamp(val, -1.0f, 1.0f); // val is between -1.0 and 1.0
            val += 1.0f; // val is between 0.0 and 2.0
            val *= 127.5f; // val is between 0.0 and 255

            // Cast to int
            uVoxelValue = static_cast<uint8_t>(val);

            //Wrte the voxel value into the volume
            volData.setVoxel(x, y, z, MaterialDensityPair88(uValue, uVoxelValue));
        }
    }
    
    template<typename MeshType>
    void Voxel::updateMesh(const MeshType& surfaceMesh) {
        // Init
        GLuint vSize = surfaceMesh.getNoOfVertices();
        GLuint iSize = surfaceMesh.getNoOfIndices();
    
        // Activate VAO
        glBindVertexArray(mData.vertexArrayObject);
    
        // Update the vertices buffer
        glBindBuffer(GL_ARRAY_BUFFER, mData.vertexBuffer);
        // If the current mesh is bigger, generate a bigger buffer
        if (vSize > mData.vBufferSize) {
            // new buffer with larger size
            glBufferData(
                GL_ARRAY_BUFFER,
                vSize * sizeof(typename MeshType::VertexType),
                surfaceMesh.getRawVertexData(),
                GL_STATIC_DRAW
            );
        
            mData.vBufferSize = vSize;
        }
        else {
            // Modifying existing buffer
            glBufferSubData(
                GL_ARRAY_BUFFER,
                0,
                vSize * sizeof(typename MeshType::VertexType),
                surfaceMesh.getRawVertexData()
            );
        }
        
        // Update the indices buffer
        glBindBuffer(GL_ARRAY_BUFFER, mData.indexBuffer);
        // Do the same for the indices buffer
        if (iSize > mData.iBufferSize) {
            glBufferData(
                GL_ELEMENT_ARRAY_BUFFER,
                surfaceMesh.getNoOfIndices() * sizeof(typename MeshType::IndexType),
                surfaceMesh.getRawIndexData(),
                GL_DYNAMIC_DRAW
            );
            
            mData.iBufferSize = iSize;
        }
        else {
            glBufferSubData(
                GL_ELEMENT_ARRAY_BUFFER,
                0,
                iSize * sizeof(typename MeshType::IndexType),
                surfaceMesh.getRawIndexData()
            );
        }
        
        // Cleanup
        glBindVertexArray(0);
        
        // Mark amount of indices to draw when rendering
        mData.noOfIndices = surfaceMesh.getNoOfIndices();
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
        // AoS organized (pos, normal, data)[]
        // The layout will be reflected in the following VertexAttribPointer calls
        glGenBuffers(1, &(meshData.vertexBuffer));
        glBindBuffer(GL_ARRAY_BUFFER, meshData.vertexBuffer);
        glBufferData(GL_ARRAY_BUFFER, surfaceMesh.getNoOfVertices() * sizeof(typename MeshType::VertexType), surfaceMesh.getRawVertexData(), GL_DYNAMIC_DRAW);

        // Create a buffer for the mesh indices
        glGenBuffers(1, &(meshData.indexBuffer));
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, meshData.indexBuffer);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, surfaceMesh.getNoOfIndices() * sizeof(typename MeshType::IndexType), surfaceMesh.getRawIndexData(), GL_DYNAMIC_DRAW);

        // Vertices layout
        glEnableVertexAttribArray(0); // Attrib '0' is the vertex positions
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(typename MeshType::VertexType), (GLvoid*)(offsetof(typename MeshType::VertexType, position))); //take the first 3 floats from every sizeof(decltype(vecVertices)::value_type)

        // Normals layout
        glEnableVertexAttribArray(1); // Attrib '1' is the vertex normals.
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(typename MeshType::VertexType), (GLvoid*)(offsetof(typename MeshType::VertexType, normal)));

        // Any extra data
        glEnableVertexAttribArray(2); // We're talking about shader attribute '2'
        // Can't upload more that 4 components (vec4 is GLSL's biggest type)
        GLint size = (std::min)(sizeof(typename MeshType::VertexType::DataType), size_t(4));
        glVertexAttribIPointer(2, size, GL_UNSIGNED_BYTE, sizeof(typename MeshType::VertexType), (GLvoid*)(offsetof(typename MeshType::VertexType, data)));

        // Reset VAO binding
        glBindVertexArray(0);

        // A few additional properties can be copied across for use during rendering.
        meshData.noOfIndices = surfaceMesh.getNoOfIndices();
        meshData.vBufferSize = surfaceMesh.getNoOfVertices();
        meshData.iBufferSize = meshData.noOfIndices;
        meshData.translation = glm::vec3(translation.getX(), translation.getY(), translation.getZ());
        meshData.scale = scale;

        // Set 16 or 32-bit index buffer size.
        meshData.indexType = sizeof(typename MeshType::IndexType) == 2 ? GL_UNSIGNED_SHORT : GL_UNSIGNED_INT;

        // Now add the mesh to the list of meshes to render.
        mData = (meshData);
    }
}
