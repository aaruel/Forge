#version 410 core

layout (location = 0) out vec3 gPosition;
layout (location = 1) out vec3 gNormal;
layout (location = 2) out vec4 gColor;

in vec2 TexCoords;
in vec3 FragPos;
in vec3 Normal;
in vec3 Tangent;
in vec3 Binormal;

uniform sampler2D texture_diffuse;
uniform sampler2D normals;

uniform mat4 view;
uniform mat4 model;

void main() {
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
    gColor = texture(texture_diffuse, TexCoords);
}
