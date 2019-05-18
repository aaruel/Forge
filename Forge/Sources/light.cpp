//
//  light.cpp
//  Forge
//
//  Created by Aaron Ruel on 5/8/19.
//

#include <stdio.h>
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
    
    // DERIVED
    // Directional Light
    
    void DirectionalLight::bind() {
        Light::bind();
        shader.bind("position", position);
    }
    
    // DERIVED
    // Point Light
    
    void PointLight::bind() {
        Light::bind();
        shader.bind("position", position).bind("viewDir", camera->getEye());
    }
    
    // DERIVED
    // Ambient Light
    
    void AmbientLight::bind() {
        Light::bind();
        shader.bind("ambientPower", ambientPower);
    }
}
