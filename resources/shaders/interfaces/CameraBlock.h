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
    glm::mat4 viewMatrix;
    glm::vec3 position;
    float exposure;
};

#else

layout(std140) uniform CameraBlock
{
    mat4 viewMatrix;
    vec3 position;
    float exposure;
} camera;

#endif  // CPP
