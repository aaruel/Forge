//
//  gui.hpp
//  Forge
//
//  Created by Aaron Ruel on 4/20/19.
//

#ifndef gui_h
#define gui_h

#include <GLFW/glfw3.h>

namespace XK { namespace GUI {
    void init(GLFWwindow *);
    void render();
    void destroy();
} }

#endif /* gui_h */
