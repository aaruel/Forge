#version 410 core

#include ../ext/ggx.glsl

out vec4 FragColor;

in vec2 TexCoords;

uniform mat4 view;
uniform vec3 position;
uniform vec3 camPos;
uniform vec3 eye;
uniform sampler2D gPosition;
uniform sampler2D gNormal;
uniform sampler2D gColor;
uniform sampler2D gSpecular;
uniform sampler2D iblbrdf;

void main() {
    // World space model vertex
    vec3 v_pos = texture(gPosition, TexCoords).rgb;

    // albedo/specular base
    vec3 base = texture(gColor, TexCoords).xyz;

    // L, V, H, N vectors
    vec3 L = normalize(position);
    vec3 V = normalize(-eye);
    vec3 H = normalize(L + V);
    vec3 N = normalize(texture(gNormal, TexCoords).rgb);

    float NdL = max(0.0  , dot(N, L));
    float NdV = max(0.001, dot(N, V));
    float NdH = max(0.001, dot(N, H));
    float HdV = max(0.001, dot(H, V));
    float LdV = max(0.001, dot(L, V));

    vec3 result = base * NdV;

    FragColor = vec4(result, 1.0);
}
