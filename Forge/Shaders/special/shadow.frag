#version 410 core

layout (location = 7) out vec3 shadowMap;

void main() {
    shadowMap = vec3(gl_FragCoord.z);
}
