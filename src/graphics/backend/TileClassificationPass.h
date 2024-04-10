/**
 * @file TileClassification.h
 * @author Ryan Purse
 * @date 09/03/2024
 */


#pragma once

#include "Context.h"
#include "FileLoader.h"
#include "GBufferFlags.h"
#include "Pch.h"
#include "Shader.h"
#include "Ubo.h"

namespace graphics
{
    /**
     * @author Ryan Purse
     * @date 09/03/2024
     */
    class TileClassificationPass
    {
    public:
        static constexpr uint32_t threadGroupSize = 16;

        TileClassificationPass();
        void execute(const glm::ivec2 &size, Context &context);

        void setUseUberVariant(bool useUber);

    protected:
        void generateShaderTable();
        static constexpr uint32_t indirectBufferSize = 4 * sizeof(uint32_t) * shaderVariationCount;

        Shader mTileShader {
            { file::shaderPath() / "classification/MaterialClassification.comp" },
            {
                { "TILE_THREAD_GROUP_SIZE", threadGroupSize },
                { "COMPUTE_SHEEN", 1 },
                { "COMPUTE_TRANSMITTANCE", 1 }
            }
        };

        std::vector<shaderVariant> mShaderTable;

        bool mUseUberVariant = false;
        Ubo mShaderTableUbo = Ubo(shaderFlagPermutations * sizeof(uint32_t));
        Ubo mUberShaderTable = Ubo(shaderFlagPermutations * sizeof(uint32_t));
    };

} // graphics
