// https://github.com/glslify/glsl-specular-cook-torrance

#include ggx.glsl

vec3 cookTorranceSpecular(
  vec3 albedo,
  vec3 radiance,
  vec3 lightDirection,
  vec3 viewDirection,
  vec3 surfaceNormal,
  float roughness,
  vec3 F0,
  float metallic
) {
  float VdotN = max(dot(viewDirection, surfaceNormal), 0.0);
  float LdotN = max(dot(lightDirection, surfaceNormal), 0.0);

  //Half angle vector
  vec3 H = normalize(lightDirection + viewDirection);

  //Geometric term (Smith)
  float NdotH = max(dot(surfaceNormal, H), 0.0);
  float VdotH = max(dot(viewDirection, H), 0.000001);
  float ggx2  = GeometrySchlickGGX(VdotN, roughness);
  float ggx1  = GeometrySchlickGGX(LdotN, roughness);
  float G = ggx1 * ggx2;

  //Distribution term (GGX)
  float D = GGX_D(surfaceNormal, H, roughness);

  //Fresnel term (Schlick)
  vec3 F = F0 + (1.0 - F0) * pow(1.0 - VdotH, 5.0);

  //Multiply terms and done
  vec3 specular = G * F * D / max(4.0 * VdotN * LdotN, 0.001);
  vec3 kS = F;
  vec3 kD = vec3(1.0) - kS;
  kD *= 1.0 - metallic;

  return (kD * albedo / PI + specular) * radiance * LdotN;
}
