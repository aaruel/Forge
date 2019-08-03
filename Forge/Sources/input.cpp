//
//  input.cpp
//  Glitter
//
//  Created by Aaron Ruel on 3/28/19.
//

#include "input.hpp"
#include "glitter.hpp"
#include <map>
#include <vector>

// static initialization
Input::MousePos Input::mousepos;
Input::MouseBut Input::mousebut;
bool Input::mouseCapture = true;
bool Input::newPos = false;
std::map<int, std::vector<std::function<void()>>> Input::keyEmitter;

void Input::mouse_callback(GLFWwindow* window, double xpos, double ypos) {
    if (!mouseCapture) return;
    // prevent mouse jumping
    if(mousepos.firstMouse) {
        mousepos.offsX = 0.0;
        mousepos.offsY = 0.0;
        mousepos.firstMouse = false;
    }
    else {
        mousepos.offsX = xpos;
        mousepos.offsY = -ypos;
    }
    glfwSetCursorPos(window, 0.0, 0.0);
    newPos = true;
}

void Input::mouse_button_callback(GLFWwindow*, int button, int action, int) {
    if (!mouseCapture) return;
    if (action == GLFW_PRESS) {
        switch (button) {
            case GLFW_MOUSE_BUTTON_LEFT:
                mousebut.leftClick = true;
                break;
            case GLFW_MOUSE_BUTTON_RIGHT:
                mousebut.rightClick = true;
                break;
            default:
                break;
        }
    }
}

void Input::key_callback(GLFWwindow *, int key, int, int action, int) {
    if (action != GLFW_PRESS) return;
    if (!mouseCapture) return;
    // Executed any hooked event callbacks
    if (keyEmitter.count(key)) {
        for (auto & function : keyEmitter[key]) {
            function();
        }
    }
}

Input::Input(GLFWwindow* _window) : window(_window) {
    glfwSetMouseButtonCallback(_window, mouse_button_callback);
    glfwSetCursorPosCallback(_window, mouse_callback);
    glfwSetKeyCallback(_window, key_callback);
    // Lock cursor to window
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
}

// Mouse

Input::MousePos Input::getMousePos() {
    return mousepos;
}

bool Input::isNewPos() {
    return newPos;
}

void Input::resetPosBool() {
    newPos = false;
}

void Input::toggleMouseCapture() {
    mouseCapture = !mouseCapture;
    if (!mouseCapture) {
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
    }
    else {
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
        glfwSetCursorPos(window, 0.0, 0.0);
    }
}

bool Input::mouseIsCaptured() {
    return mouseCapture;
}

// Keys

bool Input::isKeyPressed(int key) {
    if (!mouseCapture) return false;
    return glfwGetKey(window, key) == GLFW_PRESS;
}

void Input::registerKeyEvent(int key, std::function<void()> function) {
    keyEmitter[key].push_back(function);
}

// Mouse Buttons (there's a better way of doing this)

bool Input::isLeftClick() {
    bool poll = mousebut.leftClick;
    // preemptively reset
    mousebut.leftClick = false;
    return poll;
}

bool Input::isRightClick() {
    bool poll = mousebut.rightClick;
    mousebut.rightClick = false;
    return poll;
}
