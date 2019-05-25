//
//  light.cpp
//  Forge
//
//  Created by Aaron Ruel on 5/8/19.
//

#include <stdio.h>
#include "glitter.hpp"
#include "light.hpp"

// Types of light to consider:
//   - Ambient Light
//   - Directional Light
//   - Point Light
//   - Spotlight
//   - Area Light
//   - Volume Light

namespace XK {
    // BASE
    // Light
    
    Light::Light(std::string name, glm::vec3 _position)
        : position(_position),
        vertName("lighting/base.vert"),
        fragName("lighting/" + name + ".frag")
    {
        shader.attach(vertName).attach(fragName).link();
    }

    void Light::bind() {
        shader.activate();
    }
    
    void Light::setPosition(glm::vec3 _position) {
        position = _position;
    }
    
    Shader * Light::getShader() {
        return &shader;
    }
    
    // BASE
    // ShadowCaster
    
    ShadowCaster::ShadowCaster() : window(GlobalSingleton::getInstance()->getWindow()) {
        shadowShader.attach("shadow.vert").attach("shadow.frag").link();
        glfwGetFramebufferSize(window, &fbWidth, &fbHeight);
    
        // Generate buffers
        glGenFramebuffers(1, &framebuffer);
        glGenTextures(1, &map);
        
        // Allocate texture
        glBindTexture(GL_TEXTURE_2D, map);
        glTexImage2D(
            GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT,
            width, height, 0, GL_DEPTH_COMPONENT,
            GL_FLOAT, NULL
        );
        
        // Texture parameters
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        
        // Attach texture to framebuffer
        glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, map, 0);
        glDrawBuffer(GL_NONE);
        glReadBuffer(GL_NONE);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }
    
    // DERIVED
    // Directional Light
    
    void DirectionalLight::bind() {
        Light::bind();
        shader.bind("position", position).bind("direction", direction);
    }
    
    // DERIVED
    // Point Light
    
    void PointLight::bind() {
        Light::bind();
        shader.bind("position", position);
    }
    
    // DERIVED
    // Ambient Light
    
    void AmbientLight::bind() {
        Light::bind();
        shader.bind("ambientPower", ambientPower);
    }
    
    // DERIVED
    // Emissive Light
    
    void EmissiveLight::bind() {
        Light::bind();
    }
    
    // DERIVED
    // Spot Light
    
    void SpotLight::bind() {
        Light::bind();
        glActiveTexture(GL_TEXTURE5);
        glBindTexture(GL_TEXTURE_2D, map);
        camera.render(shader.get());
        shader
            .bind("position", position)
            .bind("direction", direction)
            .bind("aperture", aperture)
            .bind("shadowMap", 5);
    }
    
    void SpotLight::updateMatrices() {
        camera.setLocation(
            position, direction,
            aperture, (float)width/(float)height, 0.1f, 1000.f
        );
    }
    
    void SpotLight::shadowPass(Pipeline * objects) {
        glViewport(0, 0, width, height);
        glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
        
            glClear(GL_DEPTH_BUFFER_BIT);
            shadowShader.activate();
            
            for (Renderable * r : *objects) {
                Shader * sholder = r->getShader();
                Camera * cholder = r->getCamera();
                // render to texture
                r->setShader(&shadowShader);
                r->setCamera(&camera);
                r->render();
                // reset to previous shader
                r->setShader(sholder);
                r->setCamera(cholder);
            }
        
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glViewport(0, 0, fbWidth, fbHeight);
    }
}
