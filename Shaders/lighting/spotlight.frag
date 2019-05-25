#version 410 core

// how the heck do shadows work?
// take each fragment and transform it to "shadow map space"
// (the same MVP used when rendering the shadow map)
// then read from shadow map to see if that fragment is occluded

out vec4 FragColor;

in vec2 TexCoords;

uniform float aperture;
uniform vec3 position;
uniform vec3 direction;
uniform mat4 view;
uniform mat4 projection;
uniform sampler2D gPosition;
uniform sampler2D gNormal;
uniform sampler2D gColor;
uniform sampler2D shadowMap;

float LinearizeDepth(float val) {
    float n = 1.0; // camera z near
    float f = 100.0; // camera z far
    float z = val;
    return (2.0 * n) / (f + n - z * (f - n));
}

float getShadow(vec3 matPosition, vec3 normal) {
    vec4 fragPosLightSpace = projection * view * vec4(matPosition, 1.0);
    // perform perspective divide
    vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
    // transform to [0,1] range
    projCoords = projCoords * 0.5 + 0.5;
    // get closest depth value from light's perspective (using [0,1] range fragPosLight as coords)
    float closestDepth = texture(shadowMap, projCoords.xy).r;
    // get depth of current fragment from light's perspective
    float currentDepth = projCoords.z;
    // calculate bias (based on depth map resolution and slope)
    vec3 lightDir = normalize(position - matPosition);
    float bias = max(0.05 * (1.0 - dot(normal, lightDir)), 0.005);
    // check whether current frag pos is in shadow
    // float shadow = currentDepth - bias > closestDepth  ? 1.0 : 0.0;
    // PCF
    float shadow = 0.0;
    vec2 texelSize = 1.0 / textureSize(shadowMap, 0);
    for(int x = -1; x <= 1; ++x) {
        for(int y = -1; y <= 1; ++y) {
            float pcfDepth = texture(shadowMap, projCoords.xy + vec2(x, y) * texelSize).r;
            shadow += currentDepth - bias > pcfDepth  ? 1.0 : 0.0;
        }
    }
    shadow /= 9.0;
    
    // keep the shadow at 0.0 when outside the far_plane region of the light's frustum.
    if(projCoords.z > 1.0)
        shadow = 0.0;
    
    return shadow;
}

void main() {
    vec4 base = texture(gColor, TexCoords);
    float diffusePower = 0.5;

    // Get textures
    vec3 normal = normalize(texture(gNormal, TexCoords).rgb);
    vec3 matPosition = texture(gPosition, TexCoords).rgb;
    vec3 viewDir = normalize(-matPosition);
    
    // Shadow transform
    float shadow = getShadow(matPosition, normal);
    
    // get angle from eye direction to frag position
    vec3 matDirection = normalize(position - matPosition);
    vec3 nDirection = normalize(direction);
    float angle = acos(max(dot(nDirection, -matDirection), 0.0));
    
    float lightDist = distance(position, matPosition);
    float lightDist2 = lightDist * lightDist;
    
    float lighting = sqrt(diffusePower / (lightDist + lightDist2));
    
    // easing edges
    float limit = radians(aperture/2.f);
    float edge = 1 - smoothstep(limit - 0.1, limit, angle);
    
    // normal intensity
    float normalAngle = max(dot(normal, matDirection), 0.0);
    
    FragColor = base * vec4(vec3(lighting), 1.0) * (1-shadow) * edge * normalAngle;
}

