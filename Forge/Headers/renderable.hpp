//
//  renderable.hpp
//  Forge
//
//  Created by Aaron Ruel on 5/18/19.
//

#ifndef renderable_h
#define renderable_h

#include "shader.hpp"
#include "camera.hpp"
#include <vector>

namespace XK {
    class Renderable {
    public:
        virtual void render() = 0;
        
        static void renderAll(std::vector<Renderable*> * v) {
            for (Renderable * r : *v) r->render();
        }
        
        void setShader(Shader * shader) { mShader = shader; }
        Shader * getShader() { return mShader; }
        
        void setCamera(Camera * camera) { mCamera = camera; }
        Camera * getCamera() { return mCamera; }
        
    protected:
        Shader * mShader;
        Camera * mCamera;
        
    };
}

using Pipeline = std::vector<XK::Renderable*>;

#endif /* renderable_h */
