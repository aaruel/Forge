// Preprocessor Directives
#ifndef GLITTER
#define GLITTER
#pragma once

// System Headers
#include <btBulletDynamicsCommon.h>
#include <GLFW/glfw3.h>

// Define Some Constants
const int mWidth = 1280;
const int mHeight = 800;

class GlobalSingleton {
private:
    GLFWwindow * __window;
    static GlobalSingleton * instance;
    
    GlobalSingleton();
    
public:
    static GlobalSingleton * getInstance();
    void setWindow(GLFWwindow * window);
    GLFWwindow * getWindow();
};


#endif //~ Glitter Header
