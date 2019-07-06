vec3 BurleyDiffuse(vec3 diffuseColor, vec3 lightDirection, vec3 viewDirection, vec3 surfaceNormal, float roughness) {
    float NdotL = dot(lightDirection, surfaceNormal);
    float NdotV = dot(surfaceNormal, viewDirection);

    vec3 H = normalize(lightDirection + viewDirection);
    float VdotH = max(dot(viewDirection, H), 0.000001);

    float energyBias = mix(roughness, 0.0, 0.5);
    float energyFactor = mix(roughness, 1.0, 1.0 / 1.51);
    float fd90 = energyBias + 2.0 * VdotH * VdotH * roughness;
    float f0 = 1.0;
    float lightScatter = f0 + (fd90 - f0) * pow(1.0 - NdotL, 5.0);
    float viewScatter = f0 + (fd90 - f0) * pow(1.0 - NdotV, 5.0);

    return diffuseColor * lightScatter * viewScatter * energyFactor;
}
