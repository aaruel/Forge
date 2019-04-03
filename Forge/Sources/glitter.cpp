//
//  glitter.cpp
//  Glitter
//
//  Created by Aaron Ruel on 4/2/19.
//

#include "glitter.hpp"

GlobalSingleton * GlobalSingleton::instance = 0;
GlobalSingleton::GlobalSingleton() {}

GlobalSingleton * GlobalSingleton::getInstance() {
    if (instance == 0) instance = new GlobalSingleton();
    return instance;
}

void GlobalSingleton::setWindow(GLFWwindow * window) { __window = window; }
GLFWwindow * GlobalSingleton::getWindow() { return __window; }
