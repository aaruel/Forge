#version 410 core

layout (location = 0) in vec4 aPos;
layout (location = 1) in vec4 aNormal;
layout (location = 2) in vec2 aTexCoords;
layout (location = 3) in vec3 aTangent;
layout (location = 4) in vec3 aBitangent;

out vec3 FragPos;
out vec2 TexCoords;
out vec3 Normal;
out vec3 Tangent;
out vec3 Binormal;

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
    Tangent = normalize(normalMatrix * aTangent);
    Binormal = normalize(normalMatrix * aBitangent);
    
    gl_Position = projection * view * worldPos;
}
