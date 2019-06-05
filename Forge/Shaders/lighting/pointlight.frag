#version 410 core

#include ../ext/orennayar.glsl
#include ../ext/cooktorrance.glsl

out vec4 FragColor;

in vec2 TexCoords;

uniform mat4 view;
uniform vec3 position;
uniform vec3 camPos;
uniform sampler2D gPosition;
uniform sampler2D gNormal;
uniform sampler2D gColor;
uniform sampler2D gSpecular;

void main() {
    vec4 base = texture(gColor, TexCoords);

    // Get textures
    vec3 normal = normalize(texture(gNormal, TexCoords).rgb);
    vec3 matPosition = texture(gPosition, TexCoords).rgb;
    float metallic = texture(gSpecular, TexCoords).g;
    float roughness = texture(gSpecular, TexCoords).b;
    float ao = texture(gSpecular, TexCoords).a;
    
    // get angle from eye direction to frag position
    vec3 viewDirection = normalize(camPos - matPosition);
    vec3 lightDirection = normalize(position - matPosition);
    
    // normal intensity
    float dist = 20.0 / dot(position - matPosition, position - matPosition);
    float specular = cookTorranceSpecular(lightDirection, viewDirection, normal, roughness, metallic);
    float diffuse = orenNayarDiffuse(lightDirection, viewDirection, normal, roughness, 0.3);
    
    vec3 lighting = vec3((base) * (diffuse + specular) * ao * dist);
    FragColor = vec4(lighting, 1.0);
}
