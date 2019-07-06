//
//  skybox.hpp
//  Glitter
//
//  Created by Aaron Ruel on 3/31/19.
//

#ifndef skybox_h
#define skybox_h

#include "camera.hpp"
#include "shader.hpp"

// System Headers
#include <glad/glad.h>
#include <iostream>
#include <kangaru/kangaru.hpp>

#define N_SKYBOX_VERTS 108

namespace XK {
    class Skybox {
        static constexpr float skyboxVertices[N_SKYBOX_VERTS] = {
            -1.0f,  1.0f, -1.0f,
            -1.0f, -1.0f, -1.0f,
             1.0f, -1.0f, -1.0f,
             1.0f, -1.0f, -1.0f,
             1.0f,  1.0f, -1.0f,
            -1.0f,  1.0f, -1.0f,

            -1.0f, -1.0f,  1.0f,
            -1.0f, -1.0f, -1.0f,
            -1.0f,  1.0f, -1.0f,
            -1.0f,  1.0f, -1.0f,
            -1.0f,  1.0f,  1.0f,
            -1.0f, -1.0f,  1.0f,

             1.0f, -1.0f, -1.0f,
             1.0f, -1.0f,  1.0f,
             1.0f,  1.0f,  1.0f,
             1.0f,  1.0f,  1.0f,
             1.0f,  1.0f, -1.0f,
             1.0f, -1.0f, -1.0f,

            -1.0f, -1.0f,  1.0f,
            -1.0f,  1.0f,  1.0f,
             1.0f,  1.0f,  1.0f,
             1.0f,  1.0f,  1.0f,
             1.0f, -1.0f,  1.0f,
            -1.0f, -1.0f,  1.0f,

            -1.0f,  1.0f, -1.0f,
             1.0f,  1.0f, -1.0f,
             1.0f,  1.0f,  1.0f,
             1.0f,  1.0f,  1.0f,
            -1.0f,  1.0f,  1.0f,
            -1.0f,  1.0f, -1.0f,

            -1.0f, -1.0f, -1.0f,
            -1.0f, -1.0f,  1.0f,
             1.0f, -1.0f, -1.0f,
             1.0f, -1.0f, -1.0f,
            -1.0f, -1.0f,  1.0f,
             1.0f, -1.0f,  1.0f
        };
        
        unsigned int textureId;
        unsigned int BRDFmapId;
        Camera * camera;
        Shader shader;
        unsigned int skyboxArray;
        unsigned int skyboxBuffer;
    
    public:
        Skybox(std::string const & filepath, std::string const & ext = "jpg");
        void draw();
        GLuint getTextureId();
        GLuint getBRDFmapId();
        
    private:
        void loadVerts();
        void loadSkybox(std::string const & filename, std::string const & ext);
        void loadSkyboxHDR();
    };
    
    struct SkyboxService : kgr::single_service<Skybox>, kgr::supplied {};
}

#endif /* skybox_h */
