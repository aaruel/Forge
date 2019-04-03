//
//  skybox.cpp
//  Glitter
//
//  Created by Aaron Ruel on 3/31/19.
//

#include "skybox.hpp"

// System Headers
#include <stb_image.h>

namespace XK {
    void Skybox::loadVerts() {
        // Bind a Vertex Array Object
        glGenVertexArrays(1, &skyboxArray);
        glBindVertexArray(skyboxArray);

        // Copy Vertex Buffer Data
        glGenBuffers(1, &skyboxBuffer);
        glBindBuffer(GL_ARRAY_BUFFER, skyboxBuffer);
        glBufferData(
            GL_ARRAY_BUFFER,
            N_SKYBOX_VERTS * sizeof(float),
            &skyboxVertices[0],
            GL_STATIC_DRAW
        );
        
        // Activate location 0
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 3, (GLvoid *)0);
        glEnableVertexAttribArray(0); // Vertex Positions
    }

    Skybox::Skybox(std::string const & filename) : camera(Camera::getInstance()) {
        loadVerts();
    
        // Mostly obtained through https://learnopengl.com/Advanced-OpenGL/Cubemaps
        const int nFaces = 6;
        std::string location = PROJECT_SOURCE_DIR "/Skybox/" + filename;
        std::string faces[nFaces] = {
            "right.jpg",
            "left.jpg",
            "top.jpg",
            "bottom.jpg",
            "front.jpg",
            "back.jpg"
        };
        
        // Generate and load texture buffer
        glGenTextures(1, &textureId);
        glBindTexture(GL_TEXTURE_CUBE_MAP, textureId);
        
        // Load Textures
        int width, height, nrChannels;
        for (int i = 0; i < nFaces; ++i) {
        
            // Load in the image
            std::string path = location + faces[i];
            unsigned char *data = stbi_load(path.c_str(), &width, &height, &nrChannels, 0);
            
            // Upload image to GPU
            if (data) {
                glTexImage2D(
                    GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,
                    0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data
                );
                stbi_image_free(data);
            }
            else {
                std::cout << "Cubemap texture failed to load at path: " << faces[i] << std::endl;
                stbi_image_free(data);
            }
        }
        
        // Set properties
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
        
        // Load shader
        shader.attach("skybox.vert").attach("skybox.frag").link();
    }
    
    void Skybox::draw() {
        glDepthMask(GL_FALSE);
        shader.activate();
        
        glBindVertexArray(skyboxArray);
        glBindTexture(GL_TEXTURE_CUBE_MAP, textureId);
        
        // Upload camera uniforms to skybox shader
        camera->render(shader.get());
        
        glDrawArrays(GL_TRIANGLES, 0, 36);
        
        glDepthMask(GL_TRUE);
    }
}
