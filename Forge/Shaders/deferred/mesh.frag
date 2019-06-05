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

uniform sampler2D diffuse;
uniform sampler2D diffuse2;
// naming convention due to assimp... this is metal roughness
// red channel is occlusion
// green channel is metallic
// blue channel is roughness
uniform sampler2D unknown;
uniform sampler2D normals;
uniform sampler2D ambient;
uniform sampler2D emissive;
uniform samplerCube cubemap;
uniform vec3 camPos;

uniform mat4 view;
uniform mat4 model;

void main() {
    float metallic = texture(unknown, TexCoords).g;
    // store the fragment position vector in the first gbuffer texture
    gPosition = FragPos;
    // also store the per-fragment normals into the gbuffer
    mat3 tangentToWorld = mat3(
        Tangent.x, Binormal.x, Normal.x,
        Tangent.y, Binormal.y, Normal.y,
        Tangent.z, Binormal.z, Normal.z
    );
    gNormal = (texture(normals, TexCoords).rgb * 2.0 - 1.0) * tangentToWorld;
    // and the diffuse per-fragment color
    vec3 I = normalize(FragPos - camPos);
    vec3 R = reflect(I, normalize(gNormal));
    gColor = mix(texture(cubemap, R), texture(diffuse2, TexCoords), metallic);
    // Beyond here is material properties
    gSpecular = vec4(
        texture(unknown, TexCoords).rgb,
        texture(ambient, TexCoords).a
    );
    gEmissive = texture(emissive, TexCoords);
}
