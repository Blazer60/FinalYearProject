/**
 * @file GBufferFlags.h
 * @author Ryan Purse
 * @date 06/03/2024
 */


#pragma once

#define GBUFFER_FLAG_BYTE_COUNT 1

#define GBUFFER_FLAG_MATERIAL_BIT 0
#define GBUFFER_FLAG_FUZZ_BIT 1

#define SHADER_VARIANT_COUNT 2
#define SHADER_TABLE_COUNT 4

#if defined(GRAPHICS_INTERFACE)

#include <cstdint>

namespace graphics
{
    constexpr uint32_t shaderVariationCount = 2;
    enum class shaderVariant : uint8_t
    {
        UberShader, BaseShader
    };

    constexpr uint32_t shaderFlagCount = 2;
    constexpr uint32_t shaderFlagPermutations = 1 << shaderFlagCount;
    enum class ShaderFlagBit : uint8_t
    {
        MaterialBit = 1 << GBUFFER_FLAG_MATERIAL_BIT,
        SheenBit = 1 << GBUFFER_FLAG_FUZZ_BIT,
    };

    inline int operator&(const uint32_t lhs, ShaderFlagBit rhs)
    {
        return static_cast<int>(lhs & static_cast<uint32_t>(rhs));
    }
}

#endif
