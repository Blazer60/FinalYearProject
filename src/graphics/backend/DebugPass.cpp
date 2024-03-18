/**
 * @file DebugPass.cpp
 * @author Ryan Purse
 * @date 10/03/2024
 */


#include "DebugPass.h"

#include "DebugGBufferBlock.h"
#include "GBufferFlags.h"
#include "GraphicsFunctions.h"
#include "LookUpTables.h"

namespace graphics
{
    DebugPass::DebugPass()
    {
        mDebugGBufferShader.block("DebugGBufferBlock", 0);
    }

    void DebugPass::execute(
        const glm::ivec2& size,
        const Context& context,
        const std::vector<DebugQueueObject>& debugQueue,
        const std::vector<LineQueueObject>& lineQueue)
    {
        PROFILE_FUNC();

        glDisable(GL_CULL_FACE);
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

        mDebugOverlay.resize(size);
        mDebugFramebuffer.attach(&mDebugOverlay, 0);
        mDebugFramebuffer.bind();
        mDebugFramebuffer.clear(glm::vec4(0.f));
        mDebugShader.bind();

        for (const auto & [vao, count, modelMatrix, colour] : debugQueue)
        {
            mDebugShader.set("u_mvp_matrix", context.cameraViewProjectionMatrix * modelMatrix);
            mDebugShader.set("u_colour", colour);
            glBindVertexArray(vao);
            glDrawElements(GL_TRIANGLES, count, GL_UNSIGNED_INT, nullptr);
        }

        glEnable(GL_CULL_FACE);
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

        mLineShader.bind();
        mLineShader.set("u_mvp_matrix", context.cameraViewProjectionMatrix);
        glBindVertexArray(mLine.vao());
        for (const auto &[startPosition, endPosition, colour] : lineQueue)
        {
            mLineShader.set("u_locationA", startPosition);
            mLineShader.set("u_locationB", endPosition);
            mLineShader.set("u_colour",    colour);
            glDrawElements(GL_LINES, mLine.indicesCount(), GL_UNSIGNED_INT, nullptr);
        }

        mDebugFramebuffer.detach(0);
    }

    TextureBufferObject& DebugPass::tileOverlay(const glm::ivec2& size, const Context& context)
    {
        mDebugTileOverlayShader.bind();
        mDebugOverlay.clear(glm::vec4(0.f, 0.f, 0.f, 0.2f));
        mDebugTileOverlayShader.image("tileOverlay", mDebugOverlay.getId(), mDebugOverlay.getFormat(), 1, false, GL_WRITE_ONLY);

        for (int i = 0; i < shaderVariationCount; ++i)
        {
            mDebugTileOverlayShader.set("shaderIndex", i);
            dispatchComputeIndirect(context.tileClassificationStorage.getId(), 4u * sizeof(uint32_t) * i);
        }

        return mDebugOverlay;
    }

    TextureBufferObject& DebugPass::whiteFurnaceTest(const glm::ivec2& size, const Context& context, const Lut &lut)
    {
        PROFILE_FUNC();
        pushDebugGroup("White Furnace Test");

        mDebugTexture.resize(size);
        mDebugTexture.clear(glm::vec4(glm::vec3(glm::pow(0.5f, 0.45454545f)), 1.f));

        mWhiteFurnaceTestShader.bind();
        mWhiteFurnaceTestShader.image("storageGBuffer", context.gbuffer.getId(), context.gbuffer.getFormat(), 0, true, GL_READ_ONLY);
        mWhiteFurnaceTestShader.image("lighting", mDebugTexture.getId(), mDebugTexture.getFormat(), 1, false, GL_READ_WRITE);
        mWhiteFurnaceTestShader.set("depthBufferTexture", context.depthBuffer.getId(), 0);

        mWhiteFurnaceTestShader.set("directionalAlbedoLut", lut.specularDirectionalAlbedo.getId(), 1);
        mWhiteFurnaceTestShader.set("directionalAlbedoAverageLut", lut.specularDirectionalAlbedoAverage.getId(), 2);
        mWhiteFurnaceTestShader.set("missingSpecularLutTexture", lut.specularMissing.getId(), 3);

        mWhiteFurnaceTestShader.set("sheenLut", lut.sheenDirectionalAlbedo.getId(), 6);
        mWhiteFurnaceTestShader.set("sheenMissing", lut.sheenMissing.getId(), 7);

        dispatchCompute(glm::ceil(glm::vec2(size) / glm::vec2(16)));

        popDebugGroup();
        return mDebugTexture;
    }

    TextureBufferObject& DebugPass::queryGBuffer(
        const glm::ivec2 &size, const Context& context,
        gbuffer type, bool gammaCorrect, const glm::vec4& defaultValue)
    {
        PROFILE_FUNC();
        pushDebugGroup("gbuffer query");

        mDebugTexture.resize(size);
        mDebugTexture.clear();

        mDebugGBufferBlock->id = toInt(type);
        mDebugGBufferBlock->gammaCorrect = static_cast<int>(gammaCorrect);
        mDebugGBufferBlock->defaultValue = defaultValue;
        mDebugGBufferBlock.updateGlsl();
        mDebugGBufferBlock.bindToSlot(0);

        mDebugGBufferShader.bind();
        mDebugGBufferShader.image("storageGBuffer", context.gbuffer.getId(), context.gbuffer.getFormat(), 0, true, GL_READ_ONLY);
        mDebugGBufferShader.image("debug", mDebugTexture.getId(), mDebugTexture.getFormat(), 1, false, GL_WRITE_ONLY);

        dispatchCompute(glm::ceil(static_cast<glm::vec2>(size) / glm::vec2(DEBUG_GBUFFER_THREAD_SIZE)));

        popDebugGroup();

        return mDebugTexture;
    }

    const TextureBufferObject& DebugPass::getDebugOverlay()
    {
        return mDebugOverlay;
    }
} // graphics
