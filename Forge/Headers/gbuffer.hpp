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
#include "light.hpp"
#include "renderable.hpp"
#include "skybox.hpp"

// System Headers
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <functional>
#include <vector>
#include <kangaru/kangaru.hpp>

namespace XK {
    class GBuffer {
    private:
        void initTexture(GLuint * buffer, GLint index, std::function<void()> texImage);
        void initCameraPlane();
        void runLighting(Light * light);

    public:
        GBuffer(GLFWwindow * window);
        GBuffer& attach(Light * light);
        Shader * getShader();
        void engage();
        void disengage();
        void runLighting();
        void createShadowMaps(Pipeline * objects);

    public:
        // Constants
        static constexpr size_t N_ATTACHMENTS = 5;

    private:
        // Frame shaders
        Shader shader;
        std::vector<Light*> lights;
        Skybox * skybox;
        
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
        GLuint gSpecular;
        GLuint gEmissive;
        GLuint gDepth;
        GLuint attachments[N_ATTACHMENTS] = {
            GL_COLOR_ATTACHMENT0,
            GL_COLOR_ATTACHMENT1,
            GL_COLOR_ATTACHMENT2,
            GL_COLOR_ATTACHMENT3,
            GL_COLOR_ATTACHMENT4,
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
