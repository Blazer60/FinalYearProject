/**
 * @file DirectionalLightBlock.h
 * @author Ryan Purse
 * @date 07/02/2024
 */


#pragma once
#ifdef GRAPHICS_INTERFACE
#include "Pch.h"
#include <glm.hpp>

struct DirectionalLightBlock
{
    glm::vec4 direction;
    glm::vec4 intensity;
    glm::mat4 vpMatrices[16];
    float cascadeDistances[16];
    glm::vec2 bias;
    int cascadeCount;
};

#else

layout(std140) uniform DirectionalLightBlock
{
    vec3 direction;
    vec3 intensity;
    mat4 vpMatrices[16];
    vec4 cascadeDistances[4];
    vec2 bias;
    int cascadeCount;
} light;

#endif  // GRAPHICS_INTERFACE
