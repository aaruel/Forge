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
#include "input.hpp"
#include "renderable.hpp"

namespace XK {
    class Light {
    public:
        Light(std::string name, glm::vec3 _position = glm::vec3(0.0));
        
        virtual void bind();
        virtual void setPosition(glm::vec3 _position);
        void setPower(float p) { power = p; }
        Shader * getShader();
        
    protected:
        glm::vec3 position;
        float power = 0.5f;
        std::string vertName;
        std::string fragName;
        Shader shader;
        
    };
    
    class ShadowCaster {
    public:
        ShadowCaster();
    
        virtual void shadowPass(Pipeline * objects) = 0;
        
    protected:
        // Shadow map resolution
        GLint height = 1024, width = 1024;
        GLint fbWidth, fbHeight;
        
        // GL components
        GLuint framebuffer;
        GLuint map;
        
        // For adjusting the viewport
        GLFWwindow * window;
        
        // Shadow depth shader
        Shader shadowShader;
    };
    
    class DirectionalLight : public Light {
    public:
        DirectionalLight() : Light("directionallight") {
            setPower(2.0f);
            camera = Camera::getInstance();
        }
        virtual void bind();
        void setDirection(glm::vec3 d) { direction = d; }
        
    private:
        glm::vec3 direction;
        Camera * camera;
    };
    
    class PointLight : public Light {
    public:
        PointLight() : Light("pointlight"), camera(Camera::getInstance()) {}
        virtual void bind();
    
    private:
        float power = 0.5;
        Camera * camera;
    };
    
    class EmissiveLight : public Light {
    public:
        EmissiveLight() : Light("emissivelight") {}
        virtual void bind();
    };
    
    class AmbientLight : public Light {
    public:
        AmbientLight() : Light("ambientlight") {}
        virtual void bind();
        void setAmbientPower(float ap) { ambientPower = ap; }
        
    private:
        float ambientPower = 0.1;
    };
    
    class SpotLight : public Light, public ShadowCaster {
    private:
        void updateMatrices();
        
    public:
    
        SpotLight() : Light("spotlight"), ShadowCaster() { updateMatrices(); }
        virtual void bind();
        virtual void setPosition(glm::vec3 p) { position = p; updateMatrices(); }
        void setDirection(glm::vec3 d) { direction = d; updateMatrices(); }
        void setAperture(float degrees) { aperture = degrees; updateMatrices(); }
        virtual void shadowPass(Pipeline * objects);
    
    private:
        glm::vec3 direction;
        float aperture = 30; // degrees
        
        // Shadow pass matrices
        Camera camera;
    };
}

#endif /* light_h */
