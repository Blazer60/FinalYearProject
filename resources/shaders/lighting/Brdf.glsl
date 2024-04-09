#version 460 core

#include "../Maths.glsl"
#include "../brdf/Ggx.glsl"
#include "../geometry/GBuffer.glsl"
#include "../Camera.glsl"

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
    const float oneMinusRspecL = 1.f - texture(sheenAlbedoLut, vec2(nDotL, gBuffer.roughness)).r;
    const float oneMinusRspecV = 1.f - texture(sheenAlbedoLut, vec2(nDotV, gBuffer.roughness)).r;
    return min(oneMinusRspecL, oneMinusRspecV);
}

#endif

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

    return fresnel * distribution * geometry / max(4.f * vDotN * lDotN, MIN_THRESHOLD);
}

vec3 evaluateDiffuseBrdf(GBuffer gBuffer, vec3 specular)
{
    return (vec3(1.f) - specular) * gBuffer.diffuse;
}

vec3 evaluateBaseClosure(GBuffer gBuffer, vec3 fresnel, vec3 l, vec3 v, vec3 lightIntensity)
{
    const vec3 n = gBuffer.normal;
    const vec3 h = normalize(l + v);

    const float vDotN = saturate(dot(v, n));
    const float lDotN = saturate(dot(l, n));
    const float hDotN = saturate(dot(h, n));

    const vec3 specular = evaluateSpecularBrdf(gBuffer, fresnel, hDotN, vDotN, lDotN);
    const vec3 missingSpecular = evaluateMissingSpecularBrdf(gBuffer, fresnel, lDotN, vDotN);
    const vec3 fullSpecular = specular + missingSpecular;

    const vec3 diffuse = evaluateDiffuseBrdf(gBuffer, fullSpecular);

    return (fullSpecular + diffuse) * lightIntensity * lDotN;
}

vec3 evaluateTopSpecularClosure(GBuffer gBuffer, vec3 fresnel, vec3 l, vec3 v, vec3 lightIntensity)
{
    const vec3 n = gBuffer.normal;
    const vec3 h = normalize(l + v);

    const float vDotN = saturate(dot(v, n));
    const float lDotN = saturate(dot(l, n));
    const float hDotN = saturate(dot(h, n));

    const vec3 specular = evaluateSpecularBrdf(gBuffer, fresnel, hDotN, vDotN, lDotN);
    const vec3 missingSpecular = evaluateMissingSpecularBrdf(gBuffer, fresnel, lDotN, vDotN);
    const vec3 fullSpecular = specular + missingSpecular;

    return fullSpecular * lightIntensity * lDotN;
}

void evaluateTopClosure(in out vec3 colour, GBuffer gBuffer, vec3 fresnel, vec3 l, vec3 v, vec3 lightIntensity)
{
    const vec3 colourSpecular = evaluateTopSpecularClosure(gBuffer, fresnel, l, v, lightIntensity);

    const vec3 extinctionCoefficient = -log(gBuffer.transmittance + vec3(0.0001f)) / gBuffer.topThickness;
    const float distance = gBuffer.topThickness / max(0.001f, dot(gBuffer.normal, v));
    const vec3 transmittance = exp(-extinctionCoefficient * distance);

    const float alpha = gBuffer.topCoverage;

    colour = alpha * (colourSpecular + transmittance * colour) + (1.f - alpha) * colour;
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

    const float nDotL = saturate(dot(gBuffer.normal, lightDirection));
    const vec3 viewDirection = normalize(camera.position - position);

    vec3 colour = vec3(0.f);
    if (gBufferHasFlag(gBuffer, GBUFFER_FLAG_TRANSMITTANCE_BIT) == 1)
    {
//        const vec3 topIoR = indexOfRefraction(gBuffer.topSpecular);
//        const vec3 bottomIoR = indexOfRefraction(gBuffer.specular);
//        const vec3 topIoR2 = topIoR * topIoR;
//        const vec3 bottomIoR2 = bottomIoR * bottomIoR;
//
//        const vec3 reflectedFresnel = fresnelSchlick(gBuffer.topSpecular, nDotL);
//        const vec3 reflectedLightIntensity = reflectedFresnel * lightIntensity;
//
//        const vec3 refractedLightIntensity = (1.f - reflectedFresnel) * (bottomIoR2 / topIoR2) * lightIntensity;
//        const vec3 refractedLightDirection = -refractEta(lightDirection, gBuffer.normal, bottomIoR / topIoR);
//        const vec3 refractedViewDirection  = -refractEta(viewDirection, gBuffer.normal, bottomIoR / topIoR);
//        const vec3 refractedFresnel = etaToColour(topIoR, bottomIoR);
//
//        colour = evaluateBaseClosure(gBuffer, refractedFresnel, refractedLightDirection, refractedViewDirection, refractedLightIntensity);
//        evaluateTopClosure(colour, gBuffer, reflectedFresnel, lightDirection, viewDirection, reflectedLightIntensity);
    }
    else
    {
        const vec3 fresnel = fresnelSchlick(gBuffer.specular, nDotL);
        colour = evaluateBaseClosure(gBuffer, fresnel, lightDirection, viewDirection, lightIntensity);
    }

    evaluateSheenCoating(colour, gBuffer, lightDirection, viewDirection, lightIntensity);

    return colour;
}