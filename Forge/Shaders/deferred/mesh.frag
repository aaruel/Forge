#version 410 core

layout (location = 0) out vec3 gPosition;
layout (location = 1) out vec3 gNormal;
layout (location = 2) out vec4 gColor;
layout (location = 3) out vec4 gSpecular;
layout (location = 4) out vec4 gEmissive;

in vec2 TexCoords;
in vec3 FragPos;
in vec3 Normal;
in vec3 Tangent;
in vec3 Binormal;
in mat3 NormalMatrix;

uniform sampler2D diffuse;
uniform sampler2D diffuse2;
// naming convention due to assimp... this is metal roughness
// red channel is occlusion
// blue channel is metallic
// green channel is roughness
uniform sampler2D unknown;
uniform sampler2D normals;
uniform sampler2D ambient;
uniform sampler2D emissive;
uniform samplerCube cubemap;
uniform vec3 camPos;

uniform mat4 view;
uniform mat4 model;

vec3 computeNormalFromTangentSpaceNormalMap(const in vec3 tangent, const in vec3 normal, const in vec3 texnormal) {
    vec3 tang = vec3(0.0,1.0,0.0);
    if (length(tangent) != 0.0) {
        tang = normalize(tangent);
    }
    vec3 B = cross(normal, tang);
    vec3 outnormal = texnormal.x*tang + texnormal.y*B + texnormal.z*normal;
    return normalize(outnormal);
}

vec3 textureNormal(const in vec3 rgb) {
    vec3 n = normalize((rgb-vec3(0.5)));
    return n;
}

void main() {
    float metallic = texture(unknown, TexCoords).b;
    float roughness = texture(unknown, TexCoords).g;
    // store the fragment position vector in the first gbuffer texture
    gPosition = FragPos;
    // also store the per-fragment normals into the gbuffer
    vec3 normalTexture = texture(normals, TexCoords).rgb;
    if (length(normalTexture) > 0.0001) {
        vec3 adjNormal = textureNormal(normalTexture);
        gNormal = computeNormalFromTangentSpaceNormalMap(Tangent, Normal, adjNormal);
    }
    else {
        gNormal = Normal;
    }
    // and the diffuse per-fragment color
    gColor = texture(diffuse, TexCoords);
    // Beyond here is material properties
    gSpecular = vec4(
        texture(unknown, TexCoords).rgb,
        texture(ambient, TexCoords).a
    );
    gEmissive = texture(emissive, TexCoords);
}
