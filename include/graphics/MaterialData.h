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
    enum class MaskOp : uint8_t
    {
        Lerp, HeaveSide,
    };

    constexpr uint8_t passthroughFlagCount = 6;
    enum class PassthroughFlags : uint8_t
    {
        None = 0,
        Diffuse = 1 << 0,
        Specular = 1 << 1,
        Normal = 1 << 2,
        Roughness = 1 << 3,
        SheenColour = 1 << 4,
        SheenRoughness = 1 << 5,
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
            default: return "unknown";
        }
    }

    struct LayerData
    {
        glm::vec4 diffuseColour = glm::vec4(0.8f, 0.8f, 0.8f, 1.f);
        glm::vec4 specularColour = glm::vec4(0.2f, 0.2f, 0.2f, 1.f);
        glm::vec4 sheenColour = glm::vec4(0.f);
        float roughness = 0.9f;
        float sheenRoughness = 0.5f;

        int32_t diffuseTextureIndex = -1;
        int32_t specularTextureIndex = -1;

        int32_t normalTextureIndex = -1;
        int32_t roughnessTextureIndex = -1;
        int32_t sheenTextureIndex = -1;
        int32_t sheenRoughnessTextureIndex = -1;
    };

    struct MaskData
    {
        float alpha = 0.5f;
        int32_t maskTextureIndex = -1;
        uint32_t passthroughFlags = 0;
    };

    struct TextureData
    {
        uint32_t width  = 0;
        uint32_t height = 0;
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
