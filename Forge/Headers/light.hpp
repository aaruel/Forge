//
//  light.hpp
//  Forge
//
//  Created by Aaron Ruel on 5/8/19.
//
/**/
//
// Deferred shading offers a general assortment of positions, normals
// albedo, etc. among all the renderable meshes. These classes exploits the
// benefits of deferred shading.

#ifndef light_h
#define light_h

#include <glm/glm.hpp>
#include "shader.hpp"
#include "camera.hpp"

namespace XK {
    class Light {
    public:
        Light(std::string name, glm::vec3 _position = glm::vec3(0.0));
        
        virtual void bind();
        void setPosition(glm::vec3 _position);
        Shader * getShader();
        
    protected:
        glm::vec3 position;
        std::string vertName;
        std::string fragName;
        Shader shader;
        
    };
    
    class DirectionalLight : public Light {
    public:
        DirectionalLight() : Light("directionallight") {}
        virtual void bind();
    };
    
    class PointLight : public Light {
    public:
        PointLight() : Light("pointlight"), camera(Camera::getInstance()) {}
        virtual void bind();
    
    private:
        float power = 0.5;
        Camera * camera;
    };
    
    class AmbientLight : public Light {
    public:
        AmbientLight() : Light("ambientlight") {}
        virtual void bind();
        void setAmbientPower(float ap) { ambientPower = ap; }
        
    private:
        float ambientPower = 0.1;
    };
}

#endif /* light_h */
