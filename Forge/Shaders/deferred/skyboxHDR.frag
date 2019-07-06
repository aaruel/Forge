#version 410 core

layout (location = 0) out vec3 gPosition;
layout (location = 1) out vec3 gNormal;
layout (location = 2) out vec4 gColor;
layout (location = 3) out vec4 gSpecular;
layout (location = 4) out vec4 gEmissive;
layout (location = 5) out vec3 gDiffuseEnv;
layout (location = 6) out vec3 gSpecularEnv;

in vec3 TexCoords;

uniform samplerCube skybox;

void main() {
    gEmissive = texture(skybox, TexCoords);
}
