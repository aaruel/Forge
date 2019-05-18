#version 410 core
out vec4 FragColor;

in vec2 TexCoords;

uniform vec3 position;
uniform sampler2D gPosition;
uniform sampler2D gNormal;
uniform sampler2D gColor;


void main() {
    vec4 base = texture(gColor, TexCoords);
    vec3 normal = normalize(texture(gNormal, TexCoords).rgb);
    vec3 matPosition = texture(gPosition, TexCoords).rgb;
    vec3 lightDirection = normalize(position - matPosition);
    float diffuse = max(dot(normal, lightDirection), 0.0);
    vec3 lighting = vec3(diffuse);
    FragColor = base * vec4(lighting * 0.5, 1.0);
}
