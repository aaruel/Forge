#version 410 core
out vec4 FragColor;

in vec2 TexCoords;

uniform vec3 position;
uniform sampler2D gPosition;
uniform sampler2D gNormal;
uniform sampler2D gColor;

void main() {
    vec4 base = texture(gColor, TexCoords);
    float diffusePower = 0.5;

    // Get textures
    vec3 normal = normalize(texture(gNormal, TexCoords).rgb);
    vec3 matPosition = texture(gPosition, TexCoords).rgb;
    vec3 viewDir = normalize(-matPosition);
    
    // Get light direction and distance
    vec3 lightDirection = normalize(position - matPosition);
    float dist = distance(position, matPosition);
    
    lightDirection = lightDirection / dist;
    dist = dist * dist;
    
    // get diffuse
    float diffuse = max(dot(normal, lightDirection), 0.0);
    diffuse = diffuse * diffusePower / dist;
    
    // get half vector
    vec3 halfVec = normalize(lightDirection + viewDir);
    
    // Specular light calculation
    float specular = pow(max(dot(halfVec, normal), 0.0), 16) / dist;
    
    vec3 lighting = vec3(diffuse + specular);
    
    FragColor = base * vec4(lighting, 1.0);
}
