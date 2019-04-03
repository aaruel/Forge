#version 410 core

layout (location = 0) in vec4 position; // This will be the position of the vertex in model-space

// The usual matrices are provided
uniform mat4 projection;
uniform mat4 view;
uniform mat4 model;

// This will be used by the fragment shader to calculate flat-shaded normals. This is an unconventional approach
// but we use it in this example framework because not all surface extractor generate surface normals.
out vec4 worldPosition;

void main()
{
    // Standard sequence of OpenGL transformations.
    worldPosition = model * position;
    vec4 cameraPosition = view * worldPosition;
    gl_Position = projection * cameraPosition;
}
