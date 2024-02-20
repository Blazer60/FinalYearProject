/**
 * @file DebugGBufferBlock.h
 * @author Ryan Purse
 * @date 20/02/2024
 */

#pragma once

#define DEBUG_GBUFFER_THREAD_SIZE 8

#define DEBUG_GBUFFER_NORMAL        0
#define DEBUG_GBUFFER_ROUGHNESS     1
#define DEBUG_GBUFFER_DIFFUSE       2
#define DEBUG_GBUFFER_SPECULAR      3
#define DEBUG_GBUFFER_EMISSIVE      4

#ifdef GRAPHICS_INTERFACE

#include "Pch.h"
#include <glm.hpp>

struct DebugGBufferBlock
{
    glm::vec4 defaultValue;
    int id;
    int gammaCorrect;
};

#else

layout(std140) uniform DebugGBufferBlock
{
    vec4 defaultValue;
    int id;
    int gammaCorrect;
} cDebug;

#endif
