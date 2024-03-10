/**
 * @file TileClassification.cpp
 * @author Ryan Purse
 * @date 09/03/2024
 */


#include "TileClassification.h"

#include "GraphicsFunctions.h"

namespace graphics
{
    TileClassification::TileClassification()
    {
        mTileShader.block("ShaderTable", 0);
    }

    void TileClassification::execute(const glm::ivec2& size, Context& context)
    {
        PROFILE_FUNC();
        pushDebugGroup("Tile Classification");

        // todo: auto generate this.
        const std::vector<uint32_t> resetData {
            0, 1, 1, 0,
            0, 1, 1, 0,
        };

        const glm::ivec2 tileCount = glm::ceil(static_cast<glm::vec2>(size) / static_cast<float>(mTileThreadGroupSize));
        const uint32_t bufferSize = 2u * sizeof(uint32_t) * tileCount.x * tileCount.y;

        context.tileClassificationStorage.resize(indirectBufferSize + shaderVariantCount * bufferSize);
        context.tileClassificationStorage.zeroOut();
        context.tileClassificationStorage.write(resetData.data(), sizeof(uint32_t) * resetData.size());

        context.tileClassificationStorage.bindToSlot(0);

        mTileShader.bind();
        mTileShader.image("storageGBuffer", context.gbuffer.getId(), context.gbuffer.getFormat(),
            0, true, GL_WRITE_ONLY);

        glDispatchCompute(tileCount.x, tileCount.y, 1);

        popDebugGroup();
    }
}
