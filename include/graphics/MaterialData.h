/**
 * @file MaterialData.h
 * @author Ryan Purse
 * @date 13/03/2024
 */


#pragma once

#include "Pch.h"
#include "TextureArrayObject.h"

namespace graphics
{
    constexpr uint8_t maskOpCount = 2;
    enum class MaskOp : uint8_t
    {
        Lerp, Threshold,
    };

    inline std::string to_string(const MaskOp maskOp)
    {
        switch (maskOp)
        {
            case MaskOp::Lerp: return "Lerp";
            case MaskOp::Threshold: return "Threshold";
            default: return "unknown";
        }
    }

    constexpr uint16_t passthroughFlagCount = 12;
    enum class PassthroughFlags : uint16_t
    {
        None = 0,
        Diffuse = 1 << 0,
        Specular = 1 << 1,
        Normal = 1 << 2,
        Roughness = 1 << 3,
        SheenColour = 1 << 4,
        SheenRoughness = 1 << 5,
        TopSpecular = 1 << 6,
        TransmittanceColour = 1 << 7,
        TopRoughness = 1 << 8,
        TopThickness = 1 << 9,
        TopCoverage = 1 << 10,
        TopNormal = 1 << 11,
    };

    inline std::string to_string(const PassthroughFlags flag)
    {
        switch (flag)
        {
            case PassthroughFlags::None: return "None";
            case PassthroughFlags::Diffuse: return "Diffuse";
            case PassthroughFlags::Specular: return "Specular";
            case PassthroughFlags::Normal: return "Normal";
            case PassthroughFlags::Roughness: return "Roughness";
            case PassthroughFlags::SheenColour: return "Sheen Colour";
            case PassthroughFlags::SheenRoughness: return "Sheen Roughness";
            case PassthroughFlags::TopSpecular: return "Top Specular";
            case PassthroughFlags::TransmittanceColour: return "Transmittance";
            case PassthroughFlags::TopRoughness: return "Top Roughness";
            case PassthroughFlags::TopThickness: return "Thickness";
            case PassthroughFlags::TopCoverage: return "Coverage";
            case PassthroughFlags::TopNormal: return "Top Normal";
            default: return "unknown";
        }
    }

    constexpr uint8_t wrapOpCount = 2;
    enum class WrapOp : uint8_t
    {
        Repeat, ClampToEdge
    };

    inline const char* to_string(const WrapOp wrap)
    {
        switch (wrap)
        {
            case WrapOp::Repeat: return "Repeat";
            case WrapOp::ClampToEdge: return "Clamp";
            default: return "unknown";
        }
    }

    struct LayerData
    {
        glm::vec4 diffuseColour = glm::vec4(0.8f, 0.8f, 0.8f, 1.f);
        glm::vec4 specularColour = glm::vec4(0.2f, 0.2f, 0.2f, 1.f);
        glm::vec4 sheenColour = glm::vec4(0.f);
        glm::vec4 topSpecularColour = glm::vec4(0.f, 0.f, 0.f, 1.f);
        glm::vec4 transmittanceColour = glm::vec4(0.f, 0.f, 0.f, 1.f);
        float roughness = 0.9f;
        float sheenRoughness = 0.5f;
        float topRoughness = 0.2f;
        float topThickness = 0.001f;
        float topCoverage = 0.f;

        int32_t diffuseTextureIndex = -1;
        int32_t specularTextureIndex = -1;
        int32_t normalTextureIndex = -1;
        int32_t roughnessTextureIndex = -1;
        int32_t sheenTextureIndex = -1;
        int32_t sheenRoughnessTextureIndex = -1;
        int32_t metallicTextureIndex = -1;
        int32_t topSpecularColourTextureIndex = -1;
        int32_t topNormalTextureIndex = -1;
        int32_t transmittanceColourTextureIndex = -1;
        int32_t topRoughnessTextureIndex = -1;
        int32_t topThicknessTextureIndex = -1;
        int32_t topCoverageTextureIndex = -1;
        int32_t _padding01 = 100;  // Can't be a vec2 otherwise glsl will auto align to the next 16 byte boundary.
        int32_t _padding02 = 100;
    };

    struct MaskData
    {
        float alpha = 0.5f;
        int32_t maskTextureIndex = -1;
        uint32_t passthroughFlags = 0;
        uint32_t maskOp = 0;
    };

    struct TextureData
    {
        uint32_t width  = 0;
        uint32_t height = 0;
        uint32_t wrapOp = 0;  // grapihcs::WrapOp.
    };

    /**
     * @author Ryan Purse
     * @date 13/03/2024
     */
    struct MaterialData
    {
        uint32_t textureArrayId = 0;
        std::vector<TextureData> textureArrayData;
        std::vector<LayerData> layers;
        std::vector<MaskData> masks;
        // flags.
    };
} // graphics
