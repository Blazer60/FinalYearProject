/**
 * @file TileClassification.cpp
 * @author Ryan Purse
 * @date 09/03/2024
 */


#include "TileClassificationPass.h"

#include "ContainerAlgorithms.h"
#include "GraphicsFunctions.h"

namespace graphics
{
    TileClassificationPass::TileClassificationPass()
    {
        generateShaderTable();
        mTileShader.block("ShaderTable", 0);
    }

    void TileClassificationPass::execute(const glm::ivec2& size, Context& context)
    {
        PROFILE_FUNC();
        pushDebugGroup("Tile Classification");

        const std::vector pattern = { 0u, 1u, 1u, 0u };
        const std::vector resetData = containers::makePattern<uint32_t>(pattern.begin(), pattern.end(), shaderVariantCount);

        const glm::ivec2 tileCount = glm::ceil(static_cast<glm::vec2>(size) / static_cast<float>(threadGroupSize));
        const uint32_t bufferSize = sizeof(uint32_t) * tileCount.x * tileCount.y;

        context.tileClassificationStorage.resize(indirectBufferSize + shaderVariantCount * bufferSize);
        context.tileClassificationStorage.zeroOut();  // Technically don't have to do this. But it gets confusing when debugging.
        context.tileClassificationStorage.write(resetData.data(), sizeof(uint32_t) * resetData.size());

        context.tileClassificationStorage.bindToSlot(1);

        if (mUseUberVariant)
            mUberShaderTable.bindToSlot(0);
        else
            mShaderTableUbo.bindToSlot(0);

        mTileShader.bind();
        mTileShader.image("storageGBuffer", context.gbuffer.getId(), context.gbuffer.getFormat(),
            0, true, GL_WRITE_ONLY);

        glDispatchCompute(tileCount.x, tileCount.y, 1);

        popDebugGroup();
    }

    void TileClassificationPass::setUseUberVariant(const bool useUber)
    {
        mUseUberVariant = useUber;
    }

    void TileClassificationPass::generateShaderTable()
    {
        mShaderTable.reserve(shaderFlagPermutations);
        for (uint32_t flag = 0; flag < shaderFlagPermutations; ++flag)
        {
            // For now, there is no specific shader for each. Only the base shader.
            if ((flag & ShaderFlagBit::TransmittanceBit) > 0 || (flag & ShaderFlagBit::SheenBit) > 0)
                mShaderTable.push_back(shaderVariant::UberShader);
            else if ((flag & ShaderFlagBit::MaterialBit) > 0)
                mShaderTable.push_back(shaderVariant::BaseShader);
            else
                mShaderTable.push_back(shaderVariant::UberShader);
        }

        {
            std::vector<uint32_t> buffer;
            buffer.reserve(shaderFlagPermutations);
            for (auto value : mShaderTable)
                buffer.push_back(static_cast<uint32_t>(value));

            mShaderTableUbo.set(buffer.data());
        }
        {
            const std::vector<uint32_t> buffer(shaderFlagPermutations);
            mUberShaderTable.set(buffer.data());
        }
    }
}
