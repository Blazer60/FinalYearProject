#version 460 core

#include "PoolSampling.glsl"
#include "../GBuffer.glsl"
#include "GeometryData.glsl"

layout(binding = 1, std430)
buffer MaterialData
{
    LayerData[] layers;
};

// binding 2 is in PoolSampling.glsl as it is shared with Material.frag

layout(binding = 3, std430)
buffer MaskDataArray
{
    MaskData[] masks;
};

in vec2 v_uv;
in vec3 v_position_ws;
in vec3 v_normal_ws;
in mat3 v_tbn_matrix;
in vec3 v_camera_position_ts;
in vec3 v_position_ts;

vec3 blend(vec3 a, vec3 b, float textureValue, float alpha, uint operation, bool passthrough)
{
    if (passthrough)
        return a;
    if (operation == OPERATION_LERP)
        return mix(a, b, textureValue * alpha);
    else if (operation == OPERATION_THRESHOLD)
        return textureValue < alpha ? a : b;
    return vec3(1.f, 0.f, 1.f);
}

float blend(float a, float b, float textureValue, float alpha, uint operation, bool passthrough)
{
    if (passthrough)
        return a;
    if (operation == OPERATION_LERP)
        return mix(a, b, textureValue * alpha);
    else if (operation == OPERATION_THRESHOLD)
        return textureValue < alpha ? a : b;
    return -1.f;
}

void main()
{
    GBuffer gBuffer = gBufferCreate();

    LayerData material = layers[0];

    if (material.metallicTextureIndex == -1)
    {
        gBuffer.diffuse         = sampleColour(material.diffuseColour, v_uv, material.diffuseTextureIndex);
        gBuffer.specular        = sampleColour(material.specularColour, v_uv, material.specularTextureIndex);
    }
    else
    {
        const vec3 darkColour = vec3(0.04f);
        const vec3 baseColour = sampleColour(material.diffuseColour, v_uv, material.diffuseTextureIndex);
        const float metallic = sampleTexture(v_uv, material.metallicTextureIndex).r;

        gBuffer.diffuse  = mix(darkColour, baseColour, 1.f - metallic);
        gBuffer.specular = mix(darkColour, baseColour, metallic);

    }

    const vec3 rawNormal = sampleNormal(v_normal_ws, v_uv, material.normalTextureIndex, v_tbn_matrix);
    const float roughness = sampleValue(material.roughness, v_uv, material.roughnessTextureIndex);
    gBuffer.normal          = normalize(rawNormal);
    gBuffer.roughness       = computeRoughness(rawNormal, roughness);
    gBuffer.fuzzColour      = sampleColour(material.sheenColour, v_uv, material.sheenTextureIndex);
    gBuffer.fuzzRoughness   = sampleValue(material.sheenRoughness, v_uv, material.sheenRoughnessTextureIndex);

    if (dot(gBuffer.fuzzColour, gBuffer.fuzzColour) >= 0.001f)
    {
        gBufferSetFlag(gBuffer, GBUFFER_FLAG_FUZZ_BIT, 1);
    }

    // There should be one less mask that there are layers. If not, we can just skip them.
    for (int i = 0; i < min(layers.length() - 1, masks.length()); ++i)
    {
        const LayerData material = layers[i + 1];
        const MaskData mask = masks[i];
        const float textureValue = sampleMask(v_uv, mask.textureIndex).r;

        GBuffer nextGBuffer = gBufferCreate();

        if (material.metallicTextureIndex == -1)
        {
            nextGBuffer.diffuse         = sampleColour(material.diffuseColour, v_uv, material.diffuseTextureIndex);
            nextGBuffer.specular        = sampleColour(material.specularColour, v_uv, material.specularTextureIndex);
        }
        else
        {
            const vec3 darkColour = vec3(0.04f);
            const vec3 baseColour = sampleColour(material.diffuseColour, v_uv, material.diffuseTextureIndex);
            const float metallic = sampleTexture(v_uv, material.metallicTextureIndex).r;

            nextGBuffer.diffuse  = mix(darkColour, baseColour, 1.f - metallic);
            nextGBuffer.specular = mix(darkColour, baseColour, metallic);

        }

        const vec3 rawNormal = sampleNormal(v_normal_ws, v_uv, material.normalTextureIndex, v_tbn_matrix);
        const float roughness = sampleValue(material.roughness, v_uv, material.roughnessTextureIndex);
        nextGBuffer.normal          = normalize(rawNormal);
        nextGBuffer.roughness       = computeRoughness(rawNormal, roughness);
        nextGBuffer.fuzzColour      = sampleColour(material.sheenColour, v_uv, material.sheenTextureIndex);
        nextGBuffer.fuzzRoughness   = sampleValue(material.sheenRoughness, v_uv, material.sheenRoughnessTextureIndex);

        nextGBuffer.topSpecular = sampleColour(material.topSpecularColour, v_uv, material.topSpecularColourTextureIndex);
        const vec3 rawTopNormal = sampleNormal(v_normal_ws, v_uv, material.topNormalTextureIndex, v_tbn_matrix);
        const float topRoughness = sampleValue(material.topRoughness, v_uv, material.topRoughnessTextureIndex);
        nextGBuffer.topNormal = normalize(rawTopNormal);
        nextGBuffer.topRoughness = computeRoughness(rawTopNormal, topRoughness);
        nextGBuffer.transmittance = sampleColour(material.transmittanceColour, v_uv, material.transmittanceColourTextureIndex);
        nextGBuffer.topThickness = sampleValue(material.topThickness, v_uv, material.topThicknessTextureIndex);
        nextGBuffer.topCoverage = sampleValue(material.topCoverage, v_uv, material.topCoverageTextureIndex);

        // Blendables.
        gBuffer.diffuse    = blend(gBuffer.diffuse,   nextGBuffer.diffuse,   textureValue, mask.alpha, mask.operation, (mask.passthroughFlags & PASSTHROUGH_FLAG_DIFFUSE)   > 0);
        gBuffer.specular   = blend(gBuffer.specular,  nextGBuffer.specular,  textureValue, mask.alpha, mask.operation, (mask.passthroughFlags & PASSTHROUGH_FLAG_SPECULAR)  > 0);
        gBuffer.normal     = blend(gBuffer.normal,    nextGBuffer.normal,    textureValue, mask.alpha, mask.operation, (mask.passthroughFlags & PASSTHROUGH_FLAG_NORMAL)    > 0);
        gBuffer.roughness  = blend(gBuffer.roughness, nextGBuffer.roughness, textureValue, mask.alpha, mask.operation, (mask.passthroughFlags & PASSTHROUGH_FLAG_ROUGHNESS) > 0);

        gBuffer.fuzzColour    = blend(gBuffer.fuzzColour,    nextGBuffer.fuzzColour,    textureValue, mask.alpha, mask.operation, (mask.passthroughFlags & PASSTHROUGH_FLAG_SHEEN)           > 0);
        gBuffer.fuzzRoughness = blend(gBuffer.fuzzRoughness, nextGBuffer.fuzzRoughness, textureValue, mask.alpha, mask.operation, (mask.passthroughFlags & PASSTHROUGH_FLAG_SHEEN_ROUGHNESS) > 0);

        gBuffer.topSpecular     = blend(gBuffer.topSpecular,   nextGBuffer.topSpecular,   textureValue, mask.alpha, mask.operation, (mask.passthroughFlags & PASSTHROUGH_FLAG_TOP_SPECULAR)  > 0);
        gBuffer.topNormal       = blend(gBuffer.topNormal,     nextGBuffer.topNormal,     textureValue, mask.alpha, mask.operation, (mask.passthroughFlags & PASSTHROUGH_FLAG_TOP_NORMAL)    > 0);
        gBuffer.topRoughness    = blend(gBuffer.topRoughness,  nextGBuffer.topRoughness,  textureValue, mask.alpha, mask.operation, (mask.passthroughFlags & PASSTHROUGH_FLAG_TOP_ROUGHNESS) > 0);
        gBuffer.transmittance   = blend(gBuffer.transmittance, nextGBuffer.transmittance, textureValue, mask.alpha, mask.operation, (mask.passthroughFlags & PASSTHROUGH_FLAG_TRANSMITTANCE) > 0);
        gBuffer.topThickness    = blend(gBuffer.topThickness,  nextGBuffer.topThickness,  textureValue, mask.alpha, mask.operation, (mask.passthroughFlags & PASSTHROUGH_FLAG_TOP_THICKNESS) > 0);
        gBuffer.topCoverage     = blend(gBuffer.topCoverage,   nextGBuffer.topCoverage,   textureValue, mask.alpha, mask.operation, (mask.passthroughFlags & PASSTHROUGH_FLAG_TOP_COVERAGE)  > 0);

        // The normal is handled in the worst way possible. Should really be using a slerp.
        gBuffer.normal = normalize(gBuffer.normal);
    }


    if (dot(gBuffer.fuzzColour, gBuffer.fuzzColour) >= 0.001f)
    {
        gBufferSetFlag(gBuffer, GBUFFER_FLAG_FUZZ_BIT, 1);
    }

    if (dot(gBuffer.topCoverage, gBuffer.topCoverage) >= 0.001f)
    {
        gBufferSetFlag(gBuffer, GBUFFER_FLAG_TRANSMITTANCE_BIT, 1);
    }

    pushToStorageGBuffer(gBuffer, ivec2(0));
}