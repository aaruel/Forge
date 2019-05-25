#version 410 core
out vec4 FragColor;

in vec2 TexCoords;

uniform vec3 position;
uniform vec3 direction;
uniform sampler2D gPosition;
uniform sampler2D gNormal;
uniform sampler2D gColor;
uniform sampler2D gSpecular;


void main() {
    // Get textures
    vec4 base = texture(gColor, TexCoords);
    vec3 normal = normalize(texture(gNormal, TexCoords).rgb);
    vec3 matPosition = texture(gPosition, TexCoords).rgb;
    
    // vector from light to fragment
    vec3 lightDirection = normalize(direction);
    float diffuse = max(dot(normal, -lightDirection), 0.0);
    vec3 lighting = vec3(diffuse);
    FragColor = base * vec4(lighting * 0.5, 1.0);
}
