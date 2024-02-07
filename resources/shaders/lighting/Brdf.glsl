#version 460 core

const float PI = 3.14159265359f;

// Unreal's fresnel function using spherical gaussian approximation.
vec3 fresnelSchlick(float vDotH, vec3 f0)
{
    return f0 + (1.f - f0) * pow(2.f, (-5.55473f * vDotH - 6.98316f) * vDotH);
}

// Unreal's and Disney's distribution function.
float distributionGgx(float nDotH, float roughness)
{
    const float alpha = roughness * roughness;
    const float alpha2 = alpha * alpha;
    const float nDotH2 = nDotH * nDotH;
    const float denominator = nDotH2 * (alpha2 - 1.f) + 1.f;

    return alpha2 / (PI * denominator * denominator);
}

// Unreal's geometry schlick function.
float geometrySchlick(float vDotN, float k)
{
    return vDotN / (vDotN * (1.f - k) + k);
}

// Unreal's geometry smith function. Note the remapping of roughness before squaring
// should only be done on analytical lights. Remap function: (R + 1) / 2
float geometrySmith(float vDotN, float lDotN, float roughness)
{
    const float r = roughness + 1.f;
    const float k = r * r / 8.f;
    return geometrySchlick(vDotN, k) * geometrySchlick(lDotN, k);
}

struct Brdf
{
    vec3 albedo;
    vec3 f0;
    float roughness;
    float vDotN;
    float lDotN;
    float vDotH;
    float nDotH;
};

vec3 calculateIrradiance(in Brdf brdf, float metallic)
{
    const vec3 fresnel = fresnelSchlick(brdf.vDotH, brdf.f0);
    const float distribution = distributionGgx(brdf.nDotH, brdf.roughness);
    const float geometry = geometrySmith(brdf.vDotN, brdf.lDotN, brdf.roughness);

    const vec3 specular = distribution * geometry * fresnel / (4.f * brdf.vDotN * brdf.lDotN + 0.0001f);

    const vec3 kD = (vec3(1.f) - fresnel) * (1.f - metallic);
    const vec3 diffuse = kD * brdf.albedo / PI;

    const vec3 irradiance = diffuse + specular;
    return irradiance;
}