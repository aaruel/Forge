#define PI 3.1415926

// compute fresnel specular factor for given base specular and product
// product could be NdV or VdH depending on used technique
vec3 fresnel_factor(in vec3 f0, in float product) {
    return mix(f0, vec3(1.0), pow(1.01 - product, 5.0));
}

// phong (lambertian) diffuse term
float phong_diffuse() {
    return (1.0 / PI);
}

float D_GGX(in float roughness, in float NdH) {
    float m = roughness * roughness;
    float m2 = m * m;
    float d = (NdH * m2 - NdH) * NdH + 1.0;
    return m2 / (PI * d * d);
}

float G_schlick(in float roughness, in float NdV, in float NdL) {
    float k = roughness * roughness * 0.5;
    float V = NdV * (1.0 - k) + k;
    float L = NdL * (1.0 - k) + k;
    return 0.25 / (V * L);
}

vec3 cooktorrance_specular(in float NdL, in float NdV, in float NdH, in vec3 specular, in float roughness) {
    float D = D_GGX(roughness, NdH);

    float G = G_schlick(roughness, NdV, NdL);

    float rim = mix(1.0 - roughness * 0.0 * 0.9, 1.0, NdV);

    return (1.0 / rim) * specular * G * D;
}
