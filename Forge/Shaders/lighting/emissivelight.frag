#version 410 core
out vec4 FragColor;

in vec2 TexCoords;

uniform float ambientPower;
uniform sampler2D gPosition;
uniform sampler2D gNormal;
uniform sampler2D gColor;
uniform sampler2D gSpecular;
uniform sampler2D gEmissive;

void main() {
    vec4 base = texture(gEmissive, TexCoords);
    FragColor = base;
}
