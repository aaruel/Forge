#version 410 core

#include ../ext/ggx.glsl

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
    vec3 base = texture(gColor, TexCoords).rgb;

    // Get textures
    vec3 normal = normalize(texture(gNormal, TexCoords).rgb);
    vec3 matPosition = texture(gPosition, TexCoords).rgb;
    float metallic = texture(gSpecular, TexCoords).g;
    float roughness = texture(gSpecular, TexCoords).b;
    float ao = texture(gSpecular, TexCoords).a;

    vec3 N = normal;
    vec3 V = -normalize(camPos - matPosition);

    vec3 F0 = vec3(0.04);
         F0 = mix(F0, base, metallic);

    // calculate per-light radiance
    vec3 L = normalize(position - matPosition);
    vec3 H = normalize(V + L);
    float distance    = length(position - matPosition);
    float attenuation = 1.0 / (distance * distance);
    vec3 radiance     = vec3(1.0) * attenuation;

    // cook-torrance brdf
    float NDF = DistributionGGX(N, H, roughness);
    float G   = GeometrySmith(N, V, L, roughness);
    vec3 F    = fresnelSchlick(max(dot(H, V), 0.0), F0);

    vec3 kS = F;
    vec3 kD = vec3(1.0) - kS;
         kD *= 1.0 - metallic;

    vec3 numerator    = NDF * G * F;
    float denominator = 4.0 * max(dot(N, V), 0.0) * max(dot(N, L), 0.0);
    vec3 specular     = numerator / max(denominator, 0.001);

    // add to outgoing radiance Lo
    float NdotL = max(dot(N, L), 0.0);
    vec3 Lo = (kD * base / PI + specular) * radiance * NdotL;

    Lo = Lo / (Lo + vec3(1.0));
    Lo = pow(Lo, vec3(1.0/2.2));

    FragColor = vec4(Lo, 1.0);
}

//void main() {
//    vec4 base = texture(gColor, TexCoords);

//    // Get textures
//    vec3 normal = normalize(texture(gNormal, TexCoords).rgb);
//    vec3 matPosition = texture(gPosition, TexCoords).rgb;
//    float metallic = texture(gSpecular, TexCoords).g;
//    float roughness = texture(gSpecular, TexCoords).b;
//    float ao = texture(gSpecular, TexCoords).a;
    
//    // get angle from eye direction to frag position
//    vec3 viewDirection = normalize(camPos - matPosition);
//    vec3 lightDirection = normalize(position - matPosition);
//    vec3 F0 = mix(vec3(0.04), base.xyz, metallic);
    
//    // light distance
//    float atten = length(position - matPosition);
//          atten = 20.0 / (atten * atten);

//    // PBR radiance
//    vec3 radiance = vec3(1.0) * atten;

//    vec3 specular = cookTorranceSpecular(base.xyz, radiance, lightDirection, viewDirection, normal, roughness, F0, metallic);
    
//    //vec3 lighting = vec3((base) * (specular) * dist);
//    FragColor = vec4(specular, 1.0);
//}
