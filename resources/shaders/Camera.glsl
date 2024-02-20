#version 460 core

#include "interfaces/CameraBlock.h"
#include "Maths.glsl"

vec3 positionFromDepth(vec2 uv, float nonLinearDepth)
{
    // OpenGl NDC is in range [-1, 1].
    const float z = 2.f * nonLinearDepth - 1.f;
    const vec4 result = camera.inverseVpMatrix * vec4(uvToNdc(uv), z, 1.f);
    return result.xyz / result.w;
}
