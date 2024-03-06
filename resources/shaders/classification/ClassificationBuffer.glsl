#version 460 core

#include "../interfaces/GBufferFlags.h"
#include "../geometry/GBuffer.glsl"

layout(binding = 1, std430)
buffer Classification
{
    uvec4 arguments[SHADER_VARIANT_COUNT];
    uvec2 tileBuffers[][SHADER_VARIANT_COUNT];
};
