#version 460 core

#include "PoolSampling.glsl"
#include "../GBuffer.glsl"
#include "GeometryData.glsl"

layout(binding = 1, std430)
buffer MaterialBuffer
{
    LayerData material;
};

in vec2 v_uv;
in vec3 v_position_ws;
in vec3 v_normal_ws;
in mat3 v_tbn_matrix;
in vec3 v_camera_position_ts;
in vec3 v_position_ts;

void main()
{
    GBuffer gBuffer = gBufferCreate();

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
        gBufferSetFlag(gBuffer, GBUFFER_FLAG_FUZZ_BIT, 1);

    gBuffer.topSpecular = sampleColour(material.topSpecularColour, v_uv, material.topSpecularColourTextureIndex);
    const vec3 rawTopNormal = sampleNormal(v_normal_ws, v_uv, material.topNormalTextureIndex, v_tbn_matrix);
    const float topRoughness = sampleValue(material.topRoughness, v_uv, material.topRoughnessTextureIndex);
    gBuffer.topNormal = normalize(rawTopNormal);
    gBuffer.topRoughness = computeRoughness(rawTopNormal, topRoughness);
    gBuffer.transmittance = sampleColour(material.transmittanceColour, v_uv, material.transmittanceColourTextureIndex);
    gBuffer.topThickness = sampleValue(material.topThickness, v_uv, material.topThicknessTextureIndex);
    gBuffer.topCoverage = sampleValue(material.topCoverage, v_uv, material.topCoverageTextureIndex);
    if (dot(gBuffer.topCoverage, gBuffer.topCoverage) >= 0.001f)
        gBufferSetFlag(gBuffer, GBUFFER_FLAG_TRANSMITTANCE_BIT, 1);

    pushToStorageGBuffer(gBuffer, ivec2(0));
}
