#version 410 core

#include ../ext/lambert.glsl
#include ../ext/ggx.glsl

out vec4 FragColor;

in vec2 TexCoords;

uniform vec3 position;
uniform vec3 direction;
uniform vec3 camPos;
uniform float power;
uniform sampler2D gPosition;
uniform sampler2D gNormal;
uniform sampler2D gColor;
uniform sampler2D gSpecular;


void main() {
    // Get textures
    vec4 base = texture(gColor, TexCoords);
    vec3 normal = normalize(texture(gNormal, TexCoords).rgb);
    vec3 matPosition = texture(gPosition, TexCoords).rgb;
    float metallic = texture(gSpecular, TexCoords).g;
    float roughness = texture(gSpecular, TexCoords).b;
    
    // vector from light to fragment
    vec3 lightDirection = normalize(-direction);
    vec3 viewDirection = normalize(camPos - matPosition);
    
    float specular = REF_GGX(normal, viewDirection, lightDirection, roughness, 0.05);
    float diffuse = lambertDiffuse(lightDirection, normal);
    vec3 lighting = vec3(diffuse + specular);
    
    FragColor = base * vec4(lighting, 1.0) * power;
}
