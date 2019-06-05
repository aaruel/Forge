#version 410 core

#include ../ext/orennayar.glsl
#include ../ext/cooktorrance.glsl

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
    
    float specular = cookTorranceSpecular(lightDirection, viewDirection, normal, roughness, metallic);
    float diffuse = orenNayarDiffuse(lightDirection, viewDirection, normal, roughness, 0.3);
    vec3 lighting = vec3(specular + diffuse);
    
    FragColor = base * vec4(lighting, 1.0) * power;
}
