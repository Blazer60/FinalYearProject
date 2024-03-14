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

    struct TextureData
    {
        uint32_t width;
        uint32_t height;
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
        // flags.
    };
} // graphics
