#version 460 core

#include "../Maths.glsl"

vec3 ggxImportanceSample(vec2 random, vec3 N, float alpha2)
{
    const float phi = 2.f * PI * random.x;
    const float cosTheta = sqrt((1.f - random.y) / (1 + (alpha2 - 1) * random.y));
    const float sinTheta = sqrt(1.f - cosTheta * cosTheta);

    const vec3 randomDirection = vec3(cos(phi) * sinTheta, sin(phi) * sinTheta, cosTheta);

    const vec3 up = abs(N.z) < 0.999f ? vec3(0.f, 0.f, 1.f) : vec3(1.f, 0.f, 0.f);
    const vec3 tangent = normalize(cross(up, N));
    const vec3 biTangent = cross(N, tangent);

    const vec3 sampleVec = tangent * randomDirection.x + biTangent * randomDirection.y + N * randomDirection.z;
    return normalize(sampleVec);
}

// Trowbridge-reitz GGX distribution with the heaviside function removed (nDotH >= 0).
float ggxDistribution(float nDotH, float alpha2)
{
    const float nDotH2 = nDotH * nDotH;
    const float denominator = 1 + nDotH2 * (alpha2 - 1);

    return alpha2 / (PI * denominator * denominator);
}

// Geometry for an isotropic GGX distribution. s is either v or l.
float ggxGeometry1(float nDotS, float alpha2)
{
    const float nDotS2 = nDotS * nDotS;
    const float oneMinusNDotS = 1.f - nDotS;
    const float innerSqrt = 1.f + (alpha2 * oneMinusNDotS / max(nDotS2, 0.001f));
    const float lambda = (-1.f + sqrt(innerSqrt)) / 2.f;

    return 1.f / (1.f + lambda);
}

float ggxGeometry2(float nDotV, float nDotL, float alpha2)
{
    return ggxGeometry1(nDotV, alpha2) * ggxGeometry1(nDotL, alpha2);
}

vec3 ggxSpecularBrdf(vec3 fresnelColour, float nDotL, float nDotV, float nDotH, float alpha2)
{
    const float geometry     = ggxGeometry2(nDotV, nDotL, alpha2);
    const float distribution = ggxDistribution(nDotH, alpha2);

    const float denominator = 4.f * nDotL * nDotV + 0.0001f;

    return (fresnelColour * geometry * distribution) / denominator;
}
