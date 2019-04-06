//
//  input.hpp
//  Glitter
//
//  Created by Aaron Ruel on 3/28/19.
//

#ifndef input_h
#define input_h

#include <GLFW/glfw3.h>

class Input {
public:
    struct MousePos {
        double prevX;
        double prevY;
        double offsX;
        double offsY;
        bool firstMouse = true;
    };
    
    struct MouseBut {
        bool leftClick;
        bool rightClick;
    };
    
    // Mouse
    MousePos getMousePos();
    bool isNewPos();
    void resetPosBool();
    
    // Keys
    bool isKeyPressed(int);
    
    // Mouse Buttons
    bool isLeftClick();
    bool isRightClick();
    
    static Input * getInstance();
    
private:
    Input(GLFWwindow* _window);

    GLFWwindow * window;
    
    static Input * instance;
};

#endif /* input_h */
