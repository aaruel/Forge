#version 410 core

// This base lighting vertex shader refers to the framebuffer quad coordinates, leaving
// lighting shaders to be written with solely fragment shaders. Saves some redundancy.

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec2 aTexCoords;

out vec2 TexCoords;

void main() {
    TexCoords = aTexCoords;
    gl_Position = vec4(aPos, 1.0);
}

