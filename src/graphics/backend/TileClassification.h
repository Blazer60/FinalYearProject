/**
 * @file TileClassification.h
 * @author Ryan Purse
 * @date 09/03/2024
 */


#pragma once

#include "Context.h"
#include "FileLoader.h"
#include "Pch.h"
#include "Shader.h"

namespace graphics
{
    /**
     * @author Ryan Purse
     * @date 09/03/2024
     */
    class TileClassification
    {
    public:
        TileClassification();
        void execute(const glm::ivec2 &size, Context &context);
    protected:
        static constexpr uint32_t shaderVariantCount = 3;
        static constexpr uint32_t indirectBufferSize = 4 * sizeof(uint32_t) * shaderVariantCount;
        static constexpr uint32_t mTileThreadGroupSize = 16;

        Shader mTileShader {
            { file::shaderPath() / "classification/MaterialClassification.comp" }
        };
    };

} // graphics
