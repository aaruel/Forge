//
//  gbuffer.cpp
//  Forge
//
//  Created by Aaron Ruel on 5/7/19.
//

#include "gbuffer.hpp"

// System Headers
#include <iostream>

#include "shader.hpp"

namespace XK {
    /// Public
    
    GBuffer::GBuffer(GLFWwindow * window) {
        glfwGetWindowSize(window, &wWidth, &wHeight);
        glfwGetFramebufferSize(window, &fbWidth, &fbHeight);
        // setup render framebuffer
        glGenFramebuffers(1, &gFramebuffer);
        glBindFramebuffer(GL_FRAMEBUFFER, gFramebuffer);
        // setup gbuffer textures
        initTexture(&gPosition, 0, [this](){
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, wWidth, wHeight, 0, GL_RGBA, GL_FLOAT, nullptr);
        });
        initTexture(&gNormal, 1, [this](){
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, wWidth, wHeight, 0, GL_RGBA, GL_FLOAT, nullptr);
        });
        initTexture(&gColor, 2, [this](){
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, wWidth, wHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
        });
        initTexture(&gSpecular, 3, [this](){
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, wWidth, wHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
        });
        initTexture(&gEmissive, 4, [this](){
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, wWidth, wHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
        });
        // Tell OpenGL the output locations of the fragment shader
        glDrawBuffers(N_ATTACHMENTS, attachments);
        // Manually generate depth buffer into a *write only* renderbuffer for depth/stencil testing
        glGenRenderbuffers(1, &gDepth);
        glBindRenderbuffer(GL_RENDERBUFFER, gDepth);
        glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, wWidth, wHeight);
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, gDepth);
        // Ensure framebuffer is finished
        GLuint fbCompl = glCheckFramebufferStatus(GL_FRAMEBUFFER);
        if (fbCompl != GL_FRAMEBUFFER_COMPLETE) {
            std::cout << "Framebuffer not complete! " << fbCompl << std::endl;
        }
        // Init render
        initCameraPlane();
        // Generate shader
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }
    
    GBuffer& GBuffer::attach(Light * light) {
        lights.push_back(light);
        // Assign texture buffers to lighting shader
        Shader * lShader = light->getShader();
        lShader->activate();
        lShader->bind("gPosition", 0);
        lShader->bind("gNormal", 1);
        lShader->bind("gColor", 2);
        lShader->bind("gSpecular", 3);
        lShader->bind("gEmissive", 4);
        return *this;
    }

    Shader * GBuffer::getShader(){
        return &shader;
    }
    
    void GBuffer::engage() {
        glBindFramebuffer(GL_FRAMEBUFFER, gFramebuffer);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glViewport(0, 0, wWidth, wHeight);
    }
    
    void GBuffer::disengage() {
        // Copy framebuffer depth buffer to actual window
        glBindFramebuffer(GL_READ_FRAMEBUFFER, gFramebuffer);
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
        glBlitFramebuffer(
            0, 0, wWidth, wHeight,
            0, 0, wWidth, wHeight,
            GL_DEPTH_BUFFER_BIT, GL_NEAREST
        );
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        // Viewport for main window (in pixels) can be more than window size!
        glViewport(0, 0, fbWidth, fbHeight);
    }
    
    void GBuffer::runLighting() {
        glDepthMask(GL_FALSE);
        // Activate gbuffer textures
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, gPosition);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, gNormal);
        glActiveTexture(GL_TEXTURE2);
        glBindTexture(GL_TEXTURE_2D, gColor);
        glActiveTexture(GL_TEXTURE3);
        glBindTexture(GL_TEXTURE_2D, gSpecular);
        glActiveTexture(GL_TEXTURE4);
        glBindTexture(GL_TEXTURE_2D, gEmissive);
        for (Light * light : lights) {
            glEnable(GL_BLEND);
            glBlendFunc(GL_ONE, GL_ONE);
            runLighting(light);
            glDisable(GL_BLEND);
        }
        glDepthMask(GL_TRUE);
    }
    
    void GBuffer::createShadowMaps(Pipeline * objects) {
        for (Light * light : lights) {
            // will return a nullptr if not a ShadowCaster
            ShadowCaster * scl = dynamic_cast<ShadowCaster*>(light);
            if (scl) scl->shadowPass(objects);
        }
    }
    
    /// Private
    
    void GBuffer::runLighting(Light * light) {
        light->bind();
        // render to the camera plane
        glBindVertexArray(screenVAO);
        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
        glBindVertexArray(0);
    }
    
    void GBuffer::initTexture(GLuint * buffer, GLint index, std::function<void()> texImage) {
        // generate buffer
        glGenTextures(1, buffer);
        glBindTexture(GL_TEXTURE_2D, *buffer);
        // setup texture
        texImage();
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + index, GL_TEXTURE_2D, *buffer, 0);
    }
    
    void GBuffer::initCameraPlane() {        
        // init data
        glGenVertexArrays(1, &screenVAO);
        glBindVertexArray(screenVAO);
        
        glGenBuffers(1, &screenVBO);
        glBindBuffer(GL_ARRAY_BUFFER, screenVBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(screenVerts), &screenVerts, GL_STATIC_DRAW);
        
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
        
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
        // de
        glBindVertexArray(0);
    }
}

