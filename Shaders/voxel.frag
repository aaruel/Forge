#version 410 core

uniform vec3 eye;
uniform mat4 model;

// Passed in from the vertex shader
in vec4 worldPosition;
in vec4 worldNormal;
in vec4 cameraPosition;

// the color that gets written to the display
out vec4 outputColor;

void main() {
    vec3 normal = worldNormal.xyz;
    // Specular
    float strength = 0.5;
    vec3 viewDir = normalize(cameraPosition - worldPosition).xyz;
    vec3 reflectDir = reflect(-eye, normal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32);
    vec3 specular = strength * spec * vec3(1.0);
    // Ambient
    float ambient = 0.5;
    // Diffuse
    vec3 lightDir = normalize(cameraPosition - worldPosition).xyz;
    float diff = max(dot(normal, lightDir), 0.0);
    // Combine
    vec3 result = (ambient + diff + specular) * vec3(0.0, 0.8, 0.0);
    outputColor = vec4(result, 1.0);
}
