#version 410 core

uniform sampler2D gPosition;
uniform sampler2D gNormal;
uniform sampler2D gColor;
uniform sampler2D gSpecular;
uniform sampler2D iblbrdf;
uniform samplerCube radiance;
uniform samplerCube irradiance;
uniform mat4 view;
uniform vec3 camPos;

in vec2 TexCoords; // texture coords
out vec4 color;


#define PI 3.1415926
#define COOK_GGX
#define COOK


// light position
uniform vec3 position;


// handy value clamping to 0 - 1 range
float saturate(in float value)
{
    return clamp(value, 0.0, 1.0);
}


// phong (lambertian) diffuse term
float phong_diffuse()
{
    return (1.0 / PI);
}


// compute fresnel specular factor for given base specular and product
// product could be NdV or VdH depending on used technique
vec3 fresnel_factor(in vec3 f0, in float product)
{
    //return mix(f0, vec3(1.0), pow(1.01 - product, 5.0));
    return max(f0 + (1.0 - f0) * pow(1.0 - product, 5.0), 0.0);
}


// following functions are copies of UE4
// for computing cook-torrance specular lighting terms

float D_blinn(in float roughness, in float NdH)
{
    float m = roughness * roughness;
    float m2 = m * m;
    float n = 2.0 / m2 - 2.0;
    return (n + 2.0) / (2.0 * PI) * pow(NdH, n);
}

float D_beckmann(in float roughness, in float NdH)
{
    float m = roughness * roughness;
    float m2 = m * m;
    float NdH2 = NdH * NdH;
    return exp((NdH2 - 1.0) / (m2 * NdH2)) / (PI * m2 * NdH2 * NdH2);
}

float D_GGX(in float roughness, in float NdH)
{
    float m = roughness * roughness;
    float m2 = m * m;
    float d = (NdH * m2 - NdH) * NdH + 1.0;
    return m2 / (PI * d * d);
}

float G_schlick(in float roughness, in float NdV, in float NdL)
{
    float k = roughness * roughness * 0.5;
    float V = NdV * (1.0 - k) + k;
    float L = NdL * (1.0 - k) + k;
    return 0.25 / (V * L);
}


// simple phong specular calculation with normalization
vec3 phong_specular(in vec3 V, in vec3 L, in vec3 N, in vec3 specular, in float roughness)
{
    vec3 R = reflect(-L, N);
    float spec = max(0.0, dot(V, R));

    float k = 1.999 / (roughness * roughness);

    return min(1.0, 3.0 * 0.0398 * k) * pow(spec, min(10000.0, k)) * specular;
}

// simple blinn specular calculation with normalization
vec3 blinn_specular(in float NdH, in vec3 specular, in float roughness)
{
    float k = 1.999 / (roughness * roughness);

    return min(1.0, 3.0 * 0.0398 * k) * pow(NdH, min(10000.0, k)) * specular;
}

// cook-torrance specular calculation
vec3 cooktorrance_specular(in float NdL, in float NdV, in float NdH, in vec3 specular, in float roughness)
{
#ifdef COOK_BLINN
    float D = D_blinn(roughness, NdH);
#endif

#ifdef COOK_BECKMANN
    float D = D_beckmann(roughness, NdH);
#endif

#ifdef COOK_GGX
    float D = D_GGX(roughness, NdH);
#endif

    float G = G_schlick(roughness, NdV, NdL);

    float rim = mix(1.0 - roughness * texture(gSpecular, TexCoords).w * 0.9, 1.0, NdV);

    return (1.0 / rim) * specular * G * D;
}

vec4 SRGBtoLINEAR(vec4 srgbIn) {
    return vec4(pow(srgbIn.xyz, vec3(2.2)), srgbIn.w);
}

vec3 LINEARtoSRGB(vec3 color) {
    return pow(color, vec3(1.0/2.2));
}

vec2 integrateBRDF(in float roughness, in float NdV) {
    vec2 brdfSample = clamp(vec2(NdV, roughness), vec2(0.0, 0.0), vec2(1.0, 1.0));
    return SRGBtoLINEAR(texture(iblbrdf, brdfSample)).rg;
}


void main() {
    // World space model vertex
    vec3 v_pos = texture(gPosition, TexCoords).rgb;

    // point light in world space
    vec3 local_light_pos = position;

    // light attenuation
    float A = 20.0 / dot(local_light_pos - v_pos, local_light_pos - v_pos);

    // L, V, H, N vectors
    vec3 L = normalize(local_light_pos - v_pos);
    vec3 V = normalize(camPos - v_pos);
    vec3 H = normalize(L + V);
    vec3 N = normalize(texture(gNormal, TexCoords).rgb);

    // albedo/specular base
    vec3 base = SRGBtoLINEAR(texture(gColor, TexCoords)).xyz;

    // roughness
    float roughness = texture(gSpecular, TexCoords).g;

    // material params
    float metallic = texture(gSpecular, TexCoords).b;

    // mix between metal and non-metal material, for non-metal
    // constant base specular factor of 0.04 grey is used
    vec3 specularColor = mix(vec3(0.04), base, metallic);

    // diffuse IBL term
    vec3 diffuseSample = SRGBtoLINEAR( texture(irradiance, N) ).rgb;

    // specular IBL term
    vec3 refl = reflect(normalize(-V), N);
    vec3 specularSample = SRGBtoLINEAR( textureLod(radiance, refl, roughness * 5) ).rgb;

    // compute material reflectance

    float NdL = max(0.0  , dot(N, L));
    float NdV = max(0.001, dot(N, V));
    float NdH = max(0.001, dot(N, H));
    float HdV = max(0.001, dot(H, V));
    float LdV = max(0.001, dot(L, V));

#ifdef PHONG
    // specular reflectance with PHONG
    vec3 specfresnel = fresnel_factor(specularColor, NdV);
    vec3 specref = phong_specular(V, L, N, specfresnel, roughness);
#endif

#ifdef BLINN
    // specular reflectance with BLINN
    vec3 specfresnel = fresnel_factor(specularColor, HdV);
    vec3 specref = blinn_specular(NdH, specfresnel, roughness);
#endif

#ifdef COOK
    // specular reflectance with COOK-TORRANCE
    vec3 specfresnel = fresnel_factor(specularColor, HdV);
    vec3 specref = cooktorrance_specular(NdL, NdV, NdH, specfresnel, roughness);
#endif

    specref *= vec3(NdL);
    vec3 diffref = (vec3(1.0) - specfresnel) * phong_diffuse() * NdL;


    // compute lighting
    vec3 reflected_light = vec3(0);
    vec3 diffuse_light = vec3(0); // initial value == constant ambient light

    // point light
    vec3 light_color = vec3(1.0) * A;
    reflected_light += specref * light_color;
    diffuse_light += diffref * light_color;

    // IBL lighting
    vec2 brdf = integrateBRDF(roughness, NdV);
    vec3 iblspec = min(vec3(0.99), specularColor * brdf.x + brdf.y);
    reflected_light += iblspec * specularSample;
    diffuse_light += diffuseSample * (1.0 / PI);

    // final result
    vec3 result =
        diffuse_light * mix(base, vec3(0.0), metallic) +
        reflected_light;

    color = vec4(LINEARtoSRGB(result), 1.0);
}
