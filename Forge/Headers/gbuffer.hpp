//
//  gbuffer.hpp
//  Forge
//
//  Created by Aaron Ruel on 5/5/19.
//

#ifndef gbuffer_h
#define gbuffer_h

// Local Headers
#include "shader.hpp"

// System Headers
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <functional>
#include <vector>

namespace XK {
    class GBuffer {
    private:
        void initTexture(GLuint * buffer, GLint index, std::function<void()> texImage);
        void initCameraPlane();
        void runLighting(Shader * lShader);

    public:
        GBuffer(GLFWwindow * window);
        GBuffer& attach(Shader * lShader);
        Shader * getShader();
        void engage();
        void disengage();
        void runLighting();

    private:
        // Frame shaders
        Shader shader;
        std::vector<Shader*> lightingShaders;
        
        // Screen sizes
        GLint wWidth;
        GLint wHeight;
        GLint fbWidth;
        GLint fbHeight;
        
        // GBuffer addresses
        GLuint gFramebuffer;
        GLuint gPosition;
        GLuint gNormal;
        GLuint gColor;
        GLuint gDepth;
        GLuint attachments[3] = {
            GL_COLOR_ATTACHMENT0,
            GL_COLOR_ATTACHMENT1,
            GL_COLOR_ATTACHMENT2
        };
        
        // Rendering frame
        GLuint screenVAO;
        GLuint screenVBO;
        GLfloat screenVerts[24] = {
            // positions        // texture Coords
            -1.0f,  1.0f, 0.0f, 0.0f, 1.0f,
            -1.0f, -1.0f, 0.0f, 0.0f, 0.0f,
             1.0f,  1.0f, 0.0f, 1.0f, 1.0f,
             1.0f, -1.0f, 0.0f, 1.0f, 0.0f,
        };
    };
}


#endif /* gbuffer_h */
