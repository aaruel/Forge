//
//  console.cpp
//  Forge
//
//  Created by Aaron Ruel on 4/7/19.
//

#include "console.hpp"
#include "input.hpp"
#include "camera.hpp"
#include <GLFW/glfw3.h>
#include <glm/gtx/string_cast.hpp>

Console::Console() {
    gltInit();
    text = gltCreateText();
    Input::getInstance()->registerKeyEvent(GLFW_KEY_1, [this](){
        toggleDisplay();
    });
    setText("hello\nworld");
}

Console::~Console() {
    gltDeleteText(text);
    gltTerminate();
}

void Console::toggleDisplay() {
    renderable = !renderable;
}

void Console::setText(std::string itext) {
    gltSetText(text, itext.c_str());
}


void Console::getPlayer() {
    Camera * cam = Camera::getInstance();
    position
        << "Player Position: " << glm::to_string(cam->getPosition()) << std::endl
        << "Eye Position: " << glm::to_string(cam->getEye()) << std::endl;
    
}

void Console::render() {
    if (!renderable) return;
    getPlayer();
    setText(position.str());
    gltBeginDraw();
        gltColor(1.0f, 1.0f, 1.0f, 1.0f);
        gltDrawText2D(text, 20.f, 20.f, 1.5f);
    gltEndDraw();
    // Clean up
    position.str("");
    position.clear();
}
