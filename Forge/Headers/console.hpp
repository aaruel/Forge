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
};

#endif /* console_h */
