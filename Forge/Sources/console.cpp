//
//  console.cpp
//  Forge
//
//  Created by Aaron Ruel on 4/7/19.
//

#include "console.hpp"
#include "input.hpp"
#include <GLFW/glfw3.h>

Console::Console() {
    gltInit();
    text = gltCreateText();
    Input::getInstance()->registerKeyEvent(GLFW_KEY_1, [this](){
        toggleDisplay();
    });
    setText("hello");
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

void Console::render() {
    if (!renderable) return;
    gltBeginDraw();
        gltColor(1.0f, 1.0f, 1.0f, 1.0f);
        gltDrawText2D(text, 20.f, 20.f, 3.0f);
    gltEndDraw();
}
