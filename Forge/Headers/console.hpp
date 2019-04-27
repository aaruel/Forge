//
//  console.hpp
//  Forge
//
//  Created by Aaron Ruel on 4/7/19.
//

#ifndef console_h
#define console_h

// System Headers
#include <glad/glad.h>
#define GLT_IMPLEMENTATION
#include <gltext.h>
#include <iostream>
#include <sstream>
#include "sol.hpp"

class Console {
private:
    void getPlayer();
    
public:
    Console();
    ~Console();
    void render();
    void toggleDisplay();
    void setText(std::string);
    
private:
    // Start with the console hidden
    bool renderable = false;
    std::stringstream position;
    GLTtext * text;
    sol::state lua;
};

#endif /* console_h */
