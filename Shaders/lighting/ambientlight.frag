#version 410 core
out vec4 FragColor;

in vec2 TexCoords;

uniform float ambientPower;
uniform sampler2D gPosition;
uniform sampler2D gNormal;
uniform sampler2D gColor;

void main() {
    vec4 base = texture(gColor, TexCoords);
    FragColor = base * ambientPower;
}

