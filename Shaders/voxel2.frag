#version 410 core

// Interpolated values from the vertex shaders
in vec2 UV;
in vec3 normal_modelspace;
in vec3 position_worldspace;
in vec3 normal_cameraspace;
in vec3 eye_cameraspace;
in vec3 lightdirection_cameraspace;

// Ouput data
out vec3 color;

// Values that stay constant for the whole mesh.
uniform sampler2D SnowBase;
uniform sampler2D SnowNormal;
uniform vec3 camPos;
uniform mat4 view;
uniform mat4 model;

void main(){
    // Normal mapping transforms
    float textureScale = 0.1;
    vec4 snownormal_modelspace = texture(SnowNormal, position_worldspace.xz);
    vec3 snownormal_cameraspace = (model * view * snownormal_modelspace).xyz;

    // Light emission properties
    // You probably want to put them as uniforms
    vec3 LightColor = vec3(1,1,1);
    float LightPower = 50.0f;
    
    // Material properties
    vec3 MaterialDiffuseColor = texture(SnowBase, position_worldspace.xz * textureScale).rgb;
    vec3 MaterialAmbientColor = vec3(0.5) * MaterialDiffuseColor;
    vec3 MaterialSpecularColor = vec3(0.3,0.3,0.3);

    // Distance to the light
    float distance = length( camPos - position_worldspace );

    // Normal of the computed fragment, in camera space
    vec3 n = normalize( snownormal_cameraspace );
    // Direction of the light (from the fragment to the light)
    vec3 l = normalize( lightdirection_cameraspace );
    float cosTheta = clamp( dot( n,l ), 0,1 );
    
    // Eye vector (towards the camera)
    vec3 E = normalize(eye_cameraspace);
    // Blinn-Phong
    vec3 halfwayDir = normalize(l + E);
    float cosAlpha = max( dot( n,halfwayDir ), 0);
    
    color =
        // Ambient : simulates indirect lighting
        MaterialAmbientColor +
        // Diffuse : "color" of the object
        MaterialDiffuseColor * LightColor * LightPower * cosTheta / (distance*distance) +
        // Specular : reflective highlight, like a mirror
        MaterialSpecularColor * LightColor * LightPower * pow(cosAlpha,16) / (distance*distance);

}
