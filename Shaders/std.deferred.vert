#version 410 core

layout (location = 0) in vec4 aPos;
layout (location = 1) in vec4 aNormal;
layout (location = 2) in vec2 aTexCoords;

out vec3 FragPos;
out vec2 TexCoords;
out vec3 Normal;

uniform mat4 projection;
uniform mat4 view;
uniform mat4 model;

void main() {
    // transform to world space
    vec4 worldPos = model * aPos;
    FragPos = worldPos.xyz;
    TexCoords = aTexCoords;
    
    // transform to normal space
    mat3 normalMatrix = transpose(inverse(mat3(model)));
    Normal = normalize(normalMatrix * aNormal.xyz);
    
    gl_Position = projection * view * worldPos;
}
