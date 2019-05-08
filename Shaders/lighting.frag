#version 410 core
out vec4 FragColor;

in vec2 TexCoords;

uniform sampler2D gPosition;
uniform sampler2D gNormal;
uniform sampler2D gColor;

void main() {
    FragColor = texture(gNormal, TexCoords);
}

