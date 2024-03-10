/**
 * @file LookUpTables.h
 * @author Ryan Purse
 * @date 10/03/2024
 */


#pragma once

#include "Pch.h"
#include "TextureBufferObject.h"

namespace graphics
{
    struct Lut
    {
        TextureBufferObject specularDirectionalAlbedo;
        TextureBufferObject specularDirectionalAlbedoAverage ;
        TextureBufferObject specularMissing;
        TextureBufferObject ltcSheenTable;
        TextureBufferObject sheenDirectionalAlbedo;
    };

    Lut precalculateLuts(uint32_t size);

    [[nodiscard]] TextureBufferObject generateSpecularDirectionalAlbedoLut(const glm::ivec2 &size);
    [[nodiscard]] TextureBufferObject generateSpecularDirectionalAlbedoAverageLut(uint32_t size, const TextureBufferObject &specularDA);
    [[nodiscard]] TextureBufferObject generateSpecularMissingLut(const glm::ivec2& size, const TextureBufferObject &specularDA, const TextureBufferObject &specularDaAverage);
    [[nodiscard]] TextureBufferObject setupLtcSheenTable();
    [[nodiscard]] TextureBufferObject generateSheenLut(const glm::ivec2& size, const TextureBufferObject &sheenTable);
}
