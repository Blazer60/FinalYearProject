/**
 * @file PointLightBlock.h
 * @author Ryan Purse
 * @date 07/02/2024
 */


#pragma once
#ifdef GRAPHICS_INTERFACE

#include "Pch.h"
#include <glm.hpp>

struct PointLightBlock
{
    glm::mat4 mvpMatrix;
    glm::vec4 position;
    glm::vec4 intensity;
    glm::vec2 bias;
    float invSqrRadius;
    float zFar;
    float softnessRadius;
};

#else

layout(std140) uniform PointLightBlock
{
    mat4 mvpMatrix;
    vec3 position;
    vec3 intensity;
    vec2 bias;
    float invSqrRadius;
    float zFar;
    float softnessRadius;
} cLight;

#endif
