#version 460 core

#include "GeometryData.glsl"
#include "../../Colour.glsl"

layout(binding = 0) uniform sampler2DArray textures;

layout(binding = 2, std430)
buffer TextureDataArray
{
    TextureData[] textureData;
};

float sampleMask(vec2 uv, int index)
{
    if (index == -1)
        return 1.f;

    TextureData data = textureData[index];
    const vec2 maxDimensions = textureSize(textures, 0).xy;

    if (data.wrapOp == WRAP_REPEAT)
        uv = fract(uv);
    else if (data.wrapOp == WRAP_CLAMP_TO_EDGE)
        uv = clamp(uv, vec2(0.f), vec2(1.f));

    const vec2 actualUv = uv * vec2(data.width, data.height) / maxDimensions;
    return texture(textures, vec3(actualUv, index)).r;
}

vec4 sampleTexture(vec2 uv, int index)
{
    if (index == -1)
        return vec4(vec3(0.f), 1.f);

    TextureData data = textureData[index];

    if (data.wrapOp == WRAP_REPEAT)
        uv = fract(uv);
    else if (data.wrapOp == WRAP_CLAMP_TO_EDGE)
        uv = clamp(uv, vec2(0.f), vec2(1.f));

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

vec3 sampleNormal(vec3 normal, vec2 uv, int index, mat3 tbnMatrix)
{
    vec3 textureNormal = sampleTexture(uv, index).rgb;
    if (textureNormal == vec3(0.f))
        return normal;
    return normalize(tbnMatrix * (2.f * textureNormal - vec3(1.f)));
}
