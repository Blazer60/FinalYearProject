#version 460 core

#include "../interfaces/GBufferFlags.h"
#include "../geometry/GBuffer.glsl"

#if !defined(TILE_THREAD_GROUP_SIZE)
    #define TILE_THREAD_GROUP_SIZE 16
#endif

layout(binding = 1, std430)
buffer Classification
{
    uvec4 arguments[SHADER_VARIANT_COUNT];
    uint  tileBuffers[][SHADER_VARIANT_COUNT];  // [x, y] y position stored in high bits. This is done to save ~0.02ms and avoid cache misses.
};
