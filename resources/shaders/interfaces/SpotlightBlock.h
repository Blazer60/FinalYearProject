/**
 * @file SpotlightBlock.h
 * @author Ryan Purse
 * @date 07/02/2024
 */


#pragma once

#ifdef GRAPHICS_INTERFACE
#include "Pch.h"
#include <glm.hpp>

struct SpotlightBlock
{
    glm::mat4 vpMatrix;
    glm::vec4 position;
    glm::vec4 direction;
    glm::vec4 intensity;
    glm::vec2 bias;
    float invSqrRadius;
    float zFar;
    float angleScale;
    float angleOffset;
};

#else

layout(std140) uniform SpotlightBlock
{
    mat4 vpMatrix;
    vec3 position;
    vec3 direction;
    vec3 intensity;
    vec2 bias;
    float invSqrRadius;
    float zFar;
    float angleScale;
    float angleOffset;
} cLight;

#endif
