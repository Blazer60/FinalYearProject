#version 460 core

#include "interfaces/CameraBlock.h"
#include "Maths.glsl"

vec3 positionFromDepth(vec2 uv, float nonLinearDepth)
{
    // OpenGl NDC is in range [-1, 1].
    const vec3 ndc = uvToNdc(vec3(uv, nonLinearDepth));
    const vec4 result = camera.inverseVpMatrix * vec4(ndc, 1.f);
    return result.xyz / result.w;
}
