#version 410 core

#include ../ext/microfacet.glsl

out vec4 FragColor;

in vec2 TexCoords;

uniform mat4 view;
uniform vec3 position;
uniform vec3 camPos;
uniform vec3 eye;
uniform sampler2D gPosition;
uniform sampler2D gNormal;
uniform sampler2D gColor;
uniform sampler2D gSpecular;
uniform sampler2D iblbrdf;
uniform samplerCube radiance;
uniform samplerCube irradiance;
uniform samplerCube cubemap;

vec4 SRGBtoLINEAR(vec4 srgbIn) {
    return vec4(pow(srgbIn.xyz, vec3(2.2)), srgbIn.w);
}

vec3 LINEARtoSRGB(vec3 color) {
    return pow(color, vec3(1.0/2.2));
}

vec3 getIBLContribution(vec3 diffuseColor, vec3 specularColor, float roughness, vec3 n, vec3 v)
{
    float NdotV = clamp(dot(n, v), 0.0, 1.0);

    float lod = clamp(roughness * float(11), 0.0, float(11));
    vec3 reflection = normalize(reflect(-v, n));

    vec2 brdfSamplePoint = clamp(vec2(NdotV, roughness), vec2(0.0, 0.0), vec2(1.0, 1.0));
    // retrieve a scale and bias to F0. See [1], Figure 3
    vec2 brdf = texture(iblbrdf, brdfSamplePoint).rg;

    vec4 diffuseSample = texture(irradiance, n);
    vec4 specularSample = textureLod(radiance, reflection, lod);

    vec3 diffuseLight = diffuseSample.rgb;
    vec3 specularLight = specularSample.rgb;

    vec3 diffuse = diffuseLight * diffuseColor;
    vec3 specular = specularLight * (specularColor * brdf.x + brdf.y);

    return diffuse + specular;
}



void main() {
    // World space model vertex
    vec3 v_pos = texture(gPosition, TexCoords).rgb;

    // albedo/specular base
    vec3 base = texture(gColor, TexCoords).xyz;

    // specular components
    float roughness = texture(gSpecular, TexCoords).g;
    float metallic = texture(gSpecular, TexCoords).b;

    // color components
    vec3 f0 = vec3(0.04);
    vec3 diffuseColor = base * (vec3(1.0) - f0) * (1.0 - metallic);
    vec3 specularColor = mix(f0, base, metallic);

    // L, V, H, N vectors
    vec3 L = normalize(position);
    vec3 V = normalize(camPos - v_pos);
    vec3 H = normalize(L + V);
    vec3 N = normalize(texture(gNormal, TexCoords).rgb);

    float NdL = max(0.0  , dot(N, L));
    float NdV = max(0.001, dot(N, V));
    float NdH = max(0.001, dot(N, H));
    float LdH = max(0.001, dot(H, L));
    float HdV = max(0.001, dot(H, V));

    // Roughness is authored as perceptual roughness; as is convention,
    // convert to material roughness by squaring the perceptual roughness.
    float alphaRoughness = roughness * roughness;
    // Compute reflectance.
    float reflectance = max(max(specularColor.r, specularColor.g), specularColor.b);
    // Collect arguments for point light call
    vec3 specularEnvironmentR0 = specularColor.rgb;
    // Anything less than 2% is physically impossible and is instead considered to be shadowing. Compare to "Real-Time-Rendering" 4th editon on page 325.
    vec3 specularEnvironmentR90 = vec3(clamp(reflectance * 50.0, 0.0, 1.0));
    // Get vector between light position and model position for attenuation calculation
    vec3 pointToLight = position - v_pos;

    // Load for point light calculation
    AngularInfo ai = AngularInfo(
        NdL,
        NdV,
        NdH,
        LdH,
        HdV,
        vec3(0, 0, 0)
    );

    // Load for point light calculation
    MaterialInfo mi = MaterialInfo(
        roughness,
        specularEnvironmentR0,
        alphaRoughness,
        diffuseColor,
        specularEnvironmentR90,
        specularColor
    );

    vec3 ibl = getIBLContribution(diffuseColor, specularColor, roughness, N, V);
    vec3 light = applyPointLight(pointToLight, ai, mi, N, V);

    vec3 result = ibl;
    result = LINEARtoSRGB(result);

    FragColor = vec4(result, 1.0);
}
