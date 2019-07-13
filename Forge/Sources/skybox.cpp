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

    void Skybox::loadSkybox(std::string const & filename, std::string const & ext) {
        // Mostly obtained through https://learnopengl.com/Advanced-OpenGL/Cubemaps
        const int nFaces = 6;
        std::string location = PROJECT_SOURCE_DIR "/Skybox/" + filename;
        std::string faces[nFaces] = {
            "right."+ext,
            "left."+ext,
            "top."+ext,
            "bottom."+ext,
            "front."+ext,
            "back."+ext
        };

        // Generate and load texture buffer
        glGenTextures(1, &textureId);
        glBindTexture(GL_TEXTURE_CUBE_MAP, textureId);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_BASE_LEVEL, 0);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAX_LEVEL, 5);

        // Load Textures
        int width, height, nrChannels;
        for (int i = 0; i < nFaces; ++i) {

            // Load in the image
            GLenum format;
            std::string path = location + faces[i];
            unsigned char *data = stbi_load(path.c_str(), &width, &height, &nrChannels, 0);

            // Set the Correct Channel Format
            switch (nrChannels) {
                case 1 : format = GL_ALPHA;     break;
                case 2 : format = GL_LUMINANCE; break;
                case 3 : format = GL_RGB;       break;
                case 4 : format = GL_RGBA;      break;
            }

            // Upload image to GPU
            if (data) {
                glTexImage2D(
                    GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,
                    0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data
                );
                stbi_image_free(data);
            }
            else {
                std::cout << "Cubemap texture failed to load at path: " << faces[i] << std::endl;
                stbi_image_free(data);
            }
        }

        // Set properties
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
        glGenerateMipmap(GL_TEXTURE_CUBE_MAP);

        // Load shader
        shader.attach("deferred/skybox.vert").attach("deferred/skybox.frag").link();
    }

    void Skybox::loadSkyboxHDR() {


        // Load shader
        shader.attach("deferred/skyboxHDR.vert").attach("deferred/skyboxHDR.frag").link();
    }

    Skybox::Skybox(std::string const & filename, std::string const & ext) : camera(Camera::getInstance()) {
        loadVerts();
        if (ext == "hdr") loadSkyboxHDR();
        else loadSkybox(filename, ext);

        // Generate and load texture buffer
        glGenTextures(1, &BRDFmapId);
        glBindTexture(GL_TEXTURE_2D, BRDFmapId);

        // Load BRDF LUT
        GLenum format;
        std::string path = PROJECT_SOURCE_DIR "/Textures/PBR/ibl_brdf_lut.png";
        int width, height, nrChannels;
        unsigned char *data = stbi_load(path.c_str(), &width, &height, &nrChannels, 0);
        switch (nrChannels) {
            case 1 : format = GL_ALPHA;     break;
            case 2 : format = GL_LUMINANCE; break;
            case 3 : format = GL_RGB;       break;
            case 4 : format = GL_RGBA;      break;
        }

        if (data) {
            glTexImage2D(
                GL_TEXTURE_2D,
                0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data
            );
            stbi_image_free(data);
        }
        else {
            std::cout << "BRDF texture failed to load" << std::endl;
            stbi_image_free(data);
        }
        
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    }
    
    GLuint Skybox::getTextureId() { return textureId; }
    GLuint Skybox::getBRDFmapId() { return BRDFmapId; }
    
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
