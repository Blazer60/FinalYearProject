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
    glm::mat4 inverseVpMatrix;
    glm::vec3 position;
    float exposure;
    float zNear;
    float zFar;
};

#else

layout(std140) uniform CameraBlock
{
    mat4 viewMatrix;
    mat4 inverseVpMatrix;
    vec3 position;
    float exposure;
    float zNear;
    float zFar;
} camera;

#endif  // CPP
