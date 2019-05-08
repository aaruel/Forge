#version 410 core

// Interpolated values from the vertex shaders
in vec2 UV;
in vec3 normal_worldspace;
in vec3 position_worldspace;
in vec3 normal_cameraspace;
in vec3 eye_cameraspace;
in vec3 lightdirection_cameraspace;

// Ouput data
out vec3 color;

// Values that stay constant for the whole mesh.
uniform sampler2D SnowBase;
uniform sampler2D SnowNormal;
uniform sampler2D SSAO;
uniform vec3 camPos;
uniform mat4 view;
uniform mat4 model;

void main(){
    // Normal mapping transforms
    float textureScale = 0.1;
    vec4 snownormal_modelspace = texture(SnowNormal, position_worldspace.xz);
    vec3 snownormal_worldspace = (model * snownormal_modelspace).xyz;
    vec3 snownormal_cameraspace = (model * view * snownormal_modelspace).xyz;
    
    // Ambient Occulsion
    vec3 ao = texture(SSAO, position_worldspace.xz).rgb;

    // Light emission properties
    // You probably want to put them as uniforms
    vec3 LightColor = vec3(0.7);
    float LightPower = 1.0f;
    
    // Material properties
    vec3 MaterialDiffuseColorX = texture(SnowBase, position_worldspace.yz).rgb;
    vec3 MaterialDiffuseColorY = texture(SnowBase, position_worldspace.xz).rgb;
    vec3 MaterialDiffuseColorZ = texture(SnowBase, position_worldspace.xy).rgb;
    vec3 MaterialDiffuseColor = (
        MaterialDiffuseColorX * snownormal_modelspace.x
        + MaterialDiffuseColorY * snownormal_modelspace.y
        + MaterialDiffuseColorZ * snownormal_modelspace.z
    );
    vec3 MaterialAmbientColor = vec3(0.1) * MaterialDiffuseColor;
    vec3 MaterialSpecularColor = vec3(0.3,0.3,0.3);

    // Distance to the light
    float distance = length( camPos - position_worldspace );

    // Normal of the computed fragment, in world space
    vec3 n = normal_worldspace;
    // Direction of the light (from the fragment to the light)
    vec3 l = normalize(camPos - position_worldspace);
    float cosTheta = clamp( dot( n, l ), 0.0, 0.5 );
    
    // Eye vector (towards the camera)
    vec3 E = normalize(eye_cameraspace);
    // Blinn-Phong
    vec3 halfwayDir = normalize(l + E);
    float cosAlpha = max( dot( n,halfwayDir ), 0);
    
    color =
        // Ambient : simulates indirect lighting
        MaterialAmbientColor +
        // Diffuse : "color" of the object
        MaterialDiffuseColor * cosTheta;
        // Specular : reflective highlight, like a mirror
        // MaterialSpecularColor * LightColor * LightPower * pow(cosAlpha,32) / (distance*distance);
}
