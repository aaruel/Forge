#version 410 core

layout (location = 0) in vec4 aPos;
layout (location = 1) in vec4 aNormal;

out vec3 FragPos;
out vec3 Normal;

uniform mat4 projection;
uniform mat4 view;
uniform mat4 model;

void main() {
    // transform to world space
    vec4 worldPos = model * aPos;
    FragPos = worldPos.xyz;
    
    // transform to normal space
    mat3 normalMatrix = transpose(inverse(mat3(model)));
    Normal = normalize(normalMatrix * aNormal.xyz);
    
    gl_Position = projection * view * worldPos;
}
