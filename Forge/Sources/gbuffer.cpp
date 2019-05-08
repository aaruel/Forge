//
//  gbuffer.cpp
//  Forge
//
//  Created by Aaron Ruel on 5/7/19.
//

#include "gbuffer.hpp"

// System Headers
#include <iostream>

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
        // Tell OpenGL the output locations of the fragment shader
        glDrawBuffers(3, attachments);
        // Manually generate depth buffer
        glGenRenderbuffers(1, &gDepth);
        glBindRenderbuffer(GL_RENDERBUFFER, gDepth);
        glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, wWidth, wHeight);
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, gDepth);
        // Ensure framebuffer is finished
        if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
            std::cout << "Framebuffer not complete!" << std::endl;
        }
        // Init render
        initCameraPlane();
        // Generate shader
        shader.attach("std.deferred.vert").attach("std.deferred.frag").link();
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }
    
    GBuffer& GBuffer::attach(Shader * lShader) {
        lightingShaders.push_back(lShader);
        // Assign texture buffers to lighting shader
        lShader->activate();
        lShader->bind("gPosition", 0);
        lShader->bind("gNormal", 1);
        lShader->bind("gColor", 2);
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
        // Ensure the depth buffer isn't overwritten
        glDepthMask(GL_FALSE);
        for (Shader * shader : lightingShaders) {
            runLighting(shader);
        }
        glDepthMask(GL_TRUE);
    }
    
    /// Private
    
    void GBuffer::runLighting(Shader * lShader) {
        lShader->activate();
        // Activate gbuffer textures
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, gPosition);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, gNormal);
        glActiveTexture(GL_TEXTURE2);
        glBindTexture(GL_TEXTURE_2D, gColor);
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

