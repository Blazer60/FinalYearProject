#version 460 core

#include "../Maths.glsl"
#include "../brdf/Ggx.glsl"
#include "../geometry/GBuffer.glsl"
#include "../Camera.glsl"
#include "Ray.glsl"

layout(binding = 2) uniform sampler2D directionalAlbedoLut;
layout(binding = 3) uniform sampler2D directionalAlbedoAverageLut;

#if COMPUTE_SHEEN > 0
layout(binding = 4) uniform sampler2D sheenTable;
layout(binding = 5) uniform sampler2D sheenAlbedoLut;
#endif

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

#if COMPUTE_SHEEN > 0
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
    const float v0Length = max(length(v0), MIN_THRESHOLD);
    const float v0Length3 = max(v0Length * v0Length * v0Length, 0.1f * MIN_THRESHOLD);

    float d0 = distributionOriginal(v0 / v0Length);

    const float sheenValue = directionalReflectance * d0 * matDet / v0Length3;

    return gBuffer.fuzzColour * sheenValue / max(nDotV, MIN_THRESHOLD);
}

float evaluateSheenMissingScalar(GBuffer gBuffer, float nDotL, float nDotV)
{
    const float oneMinusRspecL = 1.f - texture(sheenAlbedoLut, vec2(nDotL, gBuffer.fuzzRoughness)).r;
    const float oneMinusRspecV = 1.f - texture(sheenAlbedoLut, vec2(nDotV, gBuffer.fuzzRoughness)).r;
    return min(oneMinusRspecL, oneMinusRspecV);
}

#endif

vec3 evaluateMissingSpecularBrdf(float roughness, vec3 fresnel, float lDotN, float vDotN)
{
    const vec3 averageFresnel = (20.f / 21.f) * fresnel + (1.f / 21.f);

    const float averageRspec = directionalAlbedoAverageWhite(vec2(roughness, 0.5f));
    const float oneMinusAverageRspec = 1.f - averageRspec;

    const float oneMinusRspecL = 1.f - directionalAlbedoWhite(vec2(lDotN, roughness));
    const float oneMinusRspecV = 1.f - directionalAlbedoWhite(vec2(vDotN, roughness));

    const vec3 denominator = PI * oneMinusAverageRspec * (vec3(1.f) - averageFresnel * oneMinusAverageRspec) + vec3(0.001f);

    return averageFresnel * averageRspec / denominator * oneMinusRspecL * oneMinusRspecV;
}

vec3 evaluateSpecularBrdf(float alpha2, vec3 fresnel, float hDotN, float vDotN, float lDotN)
{
    const float distribution = ggxDistribution(hDotN, alpha2);
    const float geometry = ggxGeometry2(vDotN, lDotN, alpha2);

    return fresnel * distribution * geometry / max(4.f * vDotN * lDotN, MIN_THRESHOLD);
}

vec3 evaluateDiffuseBrdf(GBuffer gBuffer, vec3 specular)
{
    return (vec3(1.f) - specular) * gBuffer.diffuse;
}

void evaluateBaseClosure(in out vec3 colour, Ray viewRay, vec3 lightDirection, GBuffer gBuffer, vec3 lightIntensity)
{
    const vec3 n = gBuffer.normal;
    const vec3 h = normalize(lightDirection + viewRay.direction);

    const float vDotN = saturate(dot(viewRay.direction, n));
    const float lDotN = saturate(dot(lightDirection, n));
    const float hDotN = saturate(dot(h, n));
    const float hDotL = saturate(dot(h, lightDirection));

    const vec3 fresnel = fresnelSchlick(gBuffer.specular, hDotL);
    const float alpha2 = gBuffer.roughness * gBuffer.roughness * gBuffer.roughness * gBuffer.roughness + 0.001f;
    const vec3 specular = evaluateSpecularBrdf(alpha2, fresnel, hDotN, vDotN, lDotN);
    const vec3 missingSpecular = evaluateMissingSpecularBrdf(gBuffer.roughness, fresnel, lDotN, vDotN);
    const vec3 fullSpecular = specular + missingSpecular;

    const vec3 diffuse = evaluateDiffuseBrdf(gBuffer, fullSpecular);

    const vec3 backColour = (fullSpecular + diffuse) * lightIntensity * lDotN;

    colour = viewRay.coverage * (colour * viewRay.transmittance * backColour) + (1.f - viewRay.coverage) * backColour;
}

#if COMPUTE_TRANSMITTANCE > 0
vec3 evaluateTopSpecularClosure(Ray viewRay, GBuffer gBuffer, vec3 lightDirection, vec3 lightIntensity)
{
    const vec3 n = gBuffer.topNormal;
    const vec3 h = normalize(lightDirection + viewRay.direction);

    const float vDotN = saturate(dot(viewRay.direction, n));
    const float lDotN = saturate(dot(lightDirection, n));
    const float hDotN = saturate(dot(h, n));
    const float hDotL = saturate(dot(h, lightDirection));

    const vec3 fresnel = fresnelSchlick(gBuffer.topSpecular, hDotL);
    const float alpha2 = gBuffer.topRoughness * gBuffer.topRoughness * gBuffer.topRoughness * gBuffer.topRoughness + 0.001f;
    const vec3 specular = evaluateSpecularBrdf(alpha2, fresnel, hDotN, vDotN, lDotN);
    const vec3 missingSpecular = evaluateMissingSpecularBrdf(gBuffer.topRoughness, fresnel, lDotN, vDotN);
    const vec3 fullSpecular = specular + missingSpecular;

    return fullSpecular * lightIntensity * lDotN;
}
#endif

void evaluateTopClosure(in out vec3 colour, in out Ray viewRay, in out vec3 lightDirection, GBuffer gBuffer, vec3 lightIntensity)
{
#if COMPUTE_TRANSMITTANCE > 0
    if (gBufferHasFlag(gBuffer, GBUFFER_FLAG_TRANSMITTANCE_BIT) == 1)
    {
        const vec3 colourSpecular = evaluateTopSpecularClosure(viewRay, gBuffer, lightDirection, lightIntensity);

        const float topThickness = mix(0.f, 0.2f, gBuffer.topThickness);
        const vec3 extinctionCoefficient = -log(gBuffer.transmittance + vec3(0.0001f)) / max(topThickness, MIN_THRESHOLD);
        const float distance = 0.01f / max(dot(gBuffer.topNormal, viewRay.direction), 0.0001f);
        // The actual thickness is a constant of 1cm.

        viewRay.transmittance = exp(-extinctionCoefficient * distance);
        viewRay.coverage = gBuffer.topCoverage;

        const float alpha = gBuffer.topCoverage;

        // Refract the view ray, change position and direction.
        const float refractiveIndex = mix(1.f, mix(1.f, 3.f, gBuffer.refractiveIndex), viewRay.coverage);
        viewRay.direction = -refract(-viewRay.direction, gBuffer.topNormal, 1.f / refractiveIndex);
        viewRay.position = viewRay.position - viewRay.direction * distance;
        viewRay.refractiveIndex = refractiveIndex;

        lightDirection = -refract(-lightDirection, gBuffer.topNormal, 1.f / refractiveIndex);
    }
#endif
}

void evaluateSheenCoating(in out vec3 colour, GBuffer gBuffer, vec3 l, vec3 v, vec3 lightIntensity)
{
#if COMPUTE_SHEEN > 0
    vec3 sheen = vec3(0.f);
    float sheenScalar = 1.f;

    const vec3 n = gBuffer.normal;
    const float vDotN = saturate(dot(v, n));
    const float lDotN = saturate(dot(l, n));

    if (gBufferHasFlag(gBuffer, GBUFFER_FLAG_FUZZ_BIT) == 1)
    {
        sheen = evalutateSheenBrdf(gBuffer, lDotN, vDotN, v);
        sheenScalar = evaluateSheenMissingScalar(gBuffer, lDotN, vDotN);
    }

    colour = sheen * lightIntensity * lDotN + sheenScalar * colour;
#endif
}

vec3 evaluateBxDF(GBuffer gBuffer, vec3 position, vec3 lightDirection, vec3 lightIntensity)
{
    if (gBufferIsValid(gBuffer) == 0)
        return vec3(0.f);

    const vec3 viewDirection = normalize(camera.position - position);

    vec3 colour = vec3(0.f);
    Ray viewRay;
    viewRay.direction = viewDirection;
    viewRay.position = position;
    viewRay.refractiveIndex = 1.f;  // Air.
    viewRay.transmittance = vec3(1.f);
    viewRay.coverage = 0.f;

    evaluateTopClosure(colour, viewRay, lightDirection, gBuffer, lightIntensity);
    evaluateBaseClosure(colour, viewRay, lightDirection, gBuffer, lightIntensity);
    evaluateSheenCoating(colour, gBuffer, lightDirection, viewDirection, lightIntensity);

    return colour;
}