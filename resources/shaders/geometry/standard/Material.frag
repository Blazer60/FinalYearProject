#version 460 core

#include "../GBuffer.glsl"
#include "../../Colour.glsl"
#include "GeometryData.glsl"

layout(binding = 0) uniform sampler2DArray textures;

layout(binding = 1, std430)
buffer MaterialData
{
    LayerData[] layers;
};

layout(binding = 2, std430)
buffer TextureDataArray
{
    TextureData[] textureData;
};

in vec2 v_uv;
in vec3 v_position_ws;
in vec3 v_normal_ws;
in mat3 v_tbn_matrix;
in vec3 v_camera_position_ts;
in vec3 v_position_ts;

vec4 sampleTexture(vec2 uv, int index)
{
    if (index == -1)
        return vec4(vec3(0.f), 1.f);

    TextureData data = textureData[index];
    const vec2 maxDimensions = textureSize(textures, 0).xy;
    const vec2 actualUv = uv * vec2(data.width, data.height) / maxDimensions;
    return texture(textures, vec3(actualUv, index));
}

vec3 sampleColour(vec4 colour, vec2 uv, int index)
{
    vec3 textureColour = sampleTexture(uv, index).rgb;
    if (textureColour == vec3(0.f))
        return sRgbToLinear(colour.rgb);
    return sRgbToLinear(colour.rgb * textureColour);
}

float sampleValue(float value, vec2 uv, int index)
{
    float textureValue = sampleTexture(uv, index).r;
    if (textureValue == 0.f)
        return value;
    return textureValue * value;
}

vec3 sampleNormal(vec3 normal, vec2 uv, int index)
{
    vec3 textureNormal = sampleTexture(uv, index).rgb;
    if (textureNormal == vec3(0.f))
        return normal;
    return normalize(v_tbn_matrix * (2.f * textureNormal - vec3(1.f)));
}

void main()
{
    GBuffer gBuffer = gBufferCreate();

    // todo: link up textures to doulbe check that they work.
    LayerData material = layers[0];

    gBuffer.diffuse         = sampleColour(material.diffuseColour, v_uv, material.diffuseTextureIndex);
    gBuffer.specular        = sampleColour(material.specularColour, v_uv, material.specularTextureIndex);
    gBuffer.normal          = sampleNormal(v_normal_ws, v_uv, material.normalTextureIndex);
    gBuffer.roughness       = sampleValue(material.roughness, v_uv, material.roughnessTextureIndex);
    gBuffer.fuzzColour      = sampleColour(material.sheenColour, v_uv, material.sheenTextureIndex);
    gBuffer.fuzzRoughness   = sampleValue(material.sheenRoughness, v_uv, material.sheenRoughnessTextureIndex);

    if (dot(gBuffer.fuzzColour, gBuffer.fuzzColour) >= 0.001f)
    {
        gBufferSetFlag(gBuffer, GBUFFER_FLAG_FUZZ_BIT, 1);
    }

    pushToStorageGBuffer(gBuffer, ivec2(0));
}