/**
 * @file DebugGBufferBlock.h
 * @author Ryan Purse
 * @date 20/02/2024
 */

#pragma once

#define DEBUG_GBUFFER_THREAD_SIZE 8

#define DEBUG_GBUFFER_NORMAL          0
#define DEBUG_GBUFFER_ROUGHNESS       1
#define DEBUG_GBUFFER_DIFFUSE         2
#define DEBUG_GBUFFER_SPECULAR        3
#define DEBUG_GBUFFER_EMISSIVE        4
#define DEBUG_GBUFFER_BYTE_COUNT      5
#define DEBUG_GBUFFER_FUZZ_COLOUR     6
#define DEBUG_GBUFFER_FUZZ_ROUGHNESS  7
#define DEBUG_GBUFFER_TOP_NORMAL      8
#define DEBUG_GBUFFER_TOP_SPECULAR    9
#define DEBUG_GBUFFER_TOP_ROUGHNESS  10
#define DEBUG_GBUFFER_TOP_COVERAGE   11
#define DEBUG_GBUFFER_TOP_THICKNESS  12
#define DEBUG_GBUFFER_TRANSMITTANCE  13

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
