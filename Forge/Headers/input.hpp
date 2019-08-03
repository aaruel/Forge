//
//  input.hpp
//  Glitter
//
//  Created by Aaron Ruel on 3/28/19.
//

#ifndef input_h
#define input_h

#include <GLFW/glfw3.h>
#include <functional>
#include <map>

class Input {
private:
    static void mouse_callback(GLFWwindow* window, double xpos, double ypos);
    static void mouse_button_callback(GLFWwindow*, int button, int action, int);
    static void key_callback(GLFWwindow*, int key, int, int action, int);

public:
    Input(GLFWwindow* _window);

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
    void toggleMouseCapture();
    bool mouseIsCaptured();
    
    // Keys
    bool isKeyPressed(int);
    void registerKeyEvent(int key, std::function<void()> function);
    
    // Mouse Buttons
    bool isLeftClick();
    bool isRightClick();
    
private:
    GLFWwindow * window;
    // Static variables because callback typing!
    static Input::MousePos mousepos;
    static Input::MouseBut mousebut;
    // Switches input contexts to GUI layer
    static bool mouseCapture;
    // Key registry
    static bool newPos;
    static std::map<int, std::vector<std::function<void()>>> keyEmitter;

};

#endif /* input_h */
