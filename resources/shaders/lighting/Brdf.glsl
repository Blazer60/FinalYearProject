#version 460 core

#include "../Maths.glsl"
#include "../brdf/Ggx.glsl"
#include "../geometry/GBuffer.glsl"
#include "../Camera.glsl"

vec3 evaluateSpecularBrdf(GBuffer gBuffer, vec3 fresnel, float hDotN, float vDotN, float lDotN)
{
    const float alpha2 = gBuffer.roughness * gBuffer.roughness * gBuffer.roughness * gBuffer.roughness + 0.001f;
    const float distribution = ggxDistribution(hDotN, alpha2);
    const float geometry = ggxGeometry2(vDotN, lDotN, alpha2);

    return fresnel * distribution * geometry / (4.f * vDotN * lDotN + 0.0001f);
}

vec3 evaluateDiffuseBrdf(GBuffer gBuffer, vec3 fresnel)
{
    return (vec3(1.f) - fresnel) * gBuffer.diffuse / PI;
}

vec3 evaluateClosure(GBuffer gBuffer, vec3 position, vec3 lightDirection)
{
    const vec3 l = lightDirection;
    const vec3 n = gBuffer.normal;
    const vec3 v = normalize(camera.position - position);
    const vec3 h = normalize(l + v);

    const float vDotN = saturate(dot(v, n));
    const float lDotN = saturate(dot(l, n));
    const float hDotN = saturate(dot(h, n));
    const float vDotH = saturate(dot(v, h));

    const vec3 fresnel = fresnelSchlick(gBuffer.specular, lDotN);
    const vec3 specular = evaluateSpecularBrdf(gBuffer, fresnel, hDotN, vDotN, lDotN);
    const vec3 diffuse = evaluateDiffuseBrdf(gBuffer, fresnel);

    return specular + diffuse;
}