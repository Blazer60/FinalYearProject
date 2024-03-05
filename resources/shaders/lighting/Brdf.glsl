#version 460 core

#include "../Maths.glsl"
#include "../brdf/Ggx.glsl"
#include "../geometry/GBuffer.glsl"
#include "../Camera.glsl"

layout(binding = 2) uniform sampler2D directionalAlbedoLut;
layout(binding = 3) uniform sampler2D directionalAlbedoAverageLut;
layout(binding = 4) uniform sampler2D sheenTable;

float directionalAlbedoWhite(vec2 uv)
{
    vec2 result = texture(directionalAlbedoLut, uv).rg;
    return result.x + result.y;
}

float directionalAlbedoAverageWhite(vec2 uv)
{
    vec2 result = texture(directionalAlbedoAverageLut, uv).rg;
    return result.x + result.y;
}

float distributionOriginal(vec3 s)
{
    return 1.f / PI * max(0.f, s.z);
}

vec3 evalutateSheenBrdf(GBuffer gBuffer, float nDotL, float nDotV, vec3 v)
{
    const float alpha = sqrt(gBuffer.fuzzRoughness);
    const vec3 tableValues = texture(sheenTable, vec2(nDotL, alpha)).rgb;
    const float a = tableValues.x;
    const float b = tableValues.y;
    const float directionalReflectance = tableValues.z;

    // [a, 0, b]
    // [0, a, 0]
    // [0, 0, 1]
    mat3 inverseMatrix;
    inverseMatrix[0][0] = a;
    inverseMatrix[1][1] = a;
    inverseMatrix[2][2] = 1.f;
    inverseMatrix[0][2] = b;

    const float matDet = determinant(inverseMatrix);

    const vec3 v0 = inverseMatrix * v;
    const float v0Length = length(v0);
    const float v0Length3 = v0Length * v0Length * v0Length;

    float d0 = distributionOriginal(v0 / v0Length);

    const float sheenValue = directionalReflectance * d0 * matDet / v0Length3;

    return gBuffer.fuzzColour * sheenValue / nDotV;
}

vec3 evaluateMissingSpecularBrdf(GBuffer gBuffer, vec3 fresnel, float lDotN, float vDotN)
{
    const vec3 averageFresnel = (20.f / 21.f) * fresnel + (1.f / 21.f);

    const float averageRspec = directionalAlbedoAverageWhite(vec2(gBuffer.roughness, 0.5f));
    const float oneMinusAverageRspec = 1.f - averageRspec;

    const float oneMinusRspecL = 1.f - directionalAlbedoWhite(vec2(lDotN, gBuffer.roughness));
    const float oneMinusRspecV = 1.f - directionalAlbedoWhite(vec2(vDotN, gBuffer.roughness));

    const vec3 denominator = PI * oneMinusAverageRspec * (vec3(1.f) - averageFresnel * oneMinusAverageRspec) + vec3(0.001f);

    return averageFresnel * averageRspec / denominator * oneMinusRspecL * oneMinusRspecV;
}

vec3 evaluateSpecularBrdf(GBuffer gBuffer, vec3 fresnel, float hDotN, float vDotN, float lDotN)
{
    const float alpha2 = gBuffer.roughness * gBuffer.roughness * gBuffer.roughness * gBuffer.roughness + 0.001f;
    const float distribution = ggxDistribution(hDotN, alpha2);
    const float geometry = ggxGeometry2(vDotN, lDotN, alpha2);

    return fresnel * distribution * geometry / (4.f * vDotN * lDotN + 0.0001f);
}

vec3 evaluateDiffuseBrdf(GBuffer gBuffer, vec3 specular)
{
    return (vec3(1.f) - specular) * gBuffer.diffuse;
}

vec3 evaluateClosure(GBuffer gBuffer, vec3 position, vec3 lightDirection)
{
    if (gBufferIsValid(gBuffer) == 0)
        return vec3(0.f);

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
    const vec3 missingSpecular = evaluateMissingSpecularBrdf(gBuffer, fresnel, lDotN, vDotN);
    const vec3 diffuse = evaluateDiffuseBrdf(gBuffer, specular + missingSpecular);

    vec3 sheen = vec3(0.f);
    if (gBufferHasFlag(gBuffer, GBUFFER_FLAG_FUZZ_BIT) == 1)
        sheen = evalutateSheenBrdf(gBuffer, lDotN, vDotN, v);

    // todo: Scale back the rest of the sheen brdf.
    return sheen + specular + missingSpecular + diffuse;
}