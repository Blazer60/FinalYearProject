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

    gBuffer.diffuse         = sampleColour(material.diffuseColour, v_uv, material.diffuseTextureIndex);
    gBuffer.specular        = sampleColour(material.specularColour, v_uv, material.specularTextureIndex);
    gBuffer.normal          = sampleNormal(v_normal_ws, v_uv, material.normalTextureIndex, v_tbn_matrix);
    gBuffer.roughness       = sampleValue(material.roughness, v_uv, material.roughnessTextureIndex);
    gBuffer.fuzzColour      = sampleColour(material.sheenColour, v_uv, material.sheenTextureIndex);
    gBuffer.fuzzRoughness   = sampleValue(material.sheenRoughness, v_uv, material.sheenRoughnessTextureIndex);

    if (dot(gBuffer.fuzzColour, gBuffer.fuzzColour) >= 0.001f)
        gBufferSetFlag(gBuffer, GBUFFER_FLAG_FUZZ_BIT, 1);

    pushToStorageGBuffer(gBuffer, ivec2(0));
}
