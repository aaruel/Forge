#version 410 core

// This will be the position of the vertex in model-space
layout (location = 0) in vec4 position;
layout (location = 1) in vec4 normal;

uniform mat4 projection;
uniform mat4 view;
uniform mat4 model;
uniform vec3 camPos;

out vec3 normal_worldspace;
out vec3 position_worldspace;
out vec3 position_cameraspace;
out vec3 eye_cameraspace;
out vec3 lightdirection_cameraspace;
out vec3 normal_cameraspace;

void main() {
    normal_worldspace = normalize(mat3(transpose(inverse(model)))*normal.xyz);
    position_worldspace = (model * position).xyz;
    position_cameraspace = (view * model * position).xyz;
    eye_cameraspace = vec3(0) - position_cameraspace;
    vec3 lightposition_cameraspace = (view * vec4(camPos, 1)).xyz;
    lightdirection_cameraspace = lightposition_cameraspace + eye_cameraspace;
    normal_cameraspace = (model * view * normal).xyz;
    gl_Position = projection * view * model * position;
}
