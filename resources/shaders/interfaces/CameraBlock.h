/**
 * @file CameraBlock.h
 * @author Ryan Purse
 * @date 05/02/2024
 */

#pragma once
#ifdef GRAPHICS_INTERFACE

#include "Pch.h"
#include <glm.hpp>

struct CameraBlock
{
    glm::vec3 position;
    float _padding;
    glm::mat4 viewMatrix;
    float exposure;
};

#else

layout(std140) uniform CameraBlock
{
    vec3 position;
    mat4 viewMatrix;
    float exposure;
} camera;

#endif  // CPP
