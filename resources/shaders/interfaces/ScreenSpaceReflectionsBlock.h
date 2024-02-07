/**
 * @file ScreenSpaceReflectionsBlock.h
 * @author Ryan Purse
 * @date 07/02/2024
 */


#pragma once

#ifdef GRAPHICS_INTERFACE
#include "Pch.h"
#include <glm.hpp>

struct ScreenSpaceReflectionsBlock
{
    glm::mat4 projection;
    float zNear;
    float luminanceMultiplier;
    float maxDistanceFalloff;
    int colourMaxLod;
};

#else

layout(std140) uniform ScreenSpaceReflectionsBlock
{
    mat4 projection;
    float zNear;
    float luminanceMultiplier;
    float maxDistanceFalloff;
    int colourMaxLod;
} cSsr;

#endif
