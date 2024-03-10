/**
 * @file RendererBackend.cpp
 * @author Ryan Purse
 * @date 09/03/2024
 */


#include "RendererBackend.h"

#include "GraphicsFunctions.h"
#include "WindowHelpers.h"

namespace graphics
{
    RendererBackend::RendererBackend()
    {
        mContext.gbuffer.setDebugName("GBuffer");
        mContext.depthBuffer.setDebugName("Depth Buffer");
        mContext.lightBuffer.setDebugName("Light Buffer");
        mContext.backBuffer.setDebugName("Back Buffer");
    }

    const Context& RendererBackend::getContext()
    {
        return mContext;
    }

    void RendererBackend::generateSkybox(const std::string_view path, const glm::ivec2& size)
    {
        mSkybox.generate(path, size);
    }

    void RendererBackend::copyQueues(Queues &&queues)
    {
        mRenderQueue = std::move(queues.renderQueue);
        mCameraQueue = std::move(queues.cameraQueue);
        mDirectionalLightQueue = std::move(queues.directionalLightQueue);
        mPointLightQueue = std::move(queues.pointLightQueue);
        mSpotlightQueue = std::move(queues.spotlightQueue);
        mDebugQueue = std::move(queues.debugQueue);
        mLineQueue = std::move(queues.lineQueue);
    }

    void RendererBackend::execute()
    {
        PROFILE_FUNC();
        pushDebugGroup("Render Pass");

        mShadowMapping.execute(mRenderQueue, mPointLightQueue);
        mShadowMapping.execute(mRenderQueue, mSpotlightQueue);

        // Reset the viewport back to the normal size once we've finished rendering all the shadows.
        glViewport(0, 0, window::bufferSize().x, window::bufferSize().y);

        for (const auto &camera : mCameraQueue)
        {
            setupCurrentCamera(camera);

            mMaterialRendering.execute(window::bufferSize(), mContext, mRenderQueue);
            mTileClassification.execute(window::bufferSize(), mContext);
            mShadowMapping.execute(camera, mRenderQueue, mDirectionalLightQueue);

            mContext.lightBuffer.resize(window::bufferSize());
            mContext.lightBuffer.clear();

            mLightShading.execute(window::bufferSize(), mContext, mPrecalcs, mDirectionalLightQueue);
            mLightShading.execute(window::bufferSize(), mContext, mPrecalcs, mPointLightQueue);
            mLightShading.execute(window::bufferSize(), mContext, mPrecalcs, mSpotlightQueue);
            mLightShading.execute(mContext, mPrecalcs, mSkybox);

            mSkyboxPass.execute(window::bufferSize(), mContext, mSkybox);

            executePostProcessStack(camera);

            // For physics debugging.
            mDebugPass.execute(window::bufferSize(), mContext, mDebugQueue, mLineQueue);
        }

        popDebugGroup();
    }

    void RendererBackend::setIblMultiplier(const float multiplier)
    {
        mSkybox.luminanceMultiplier = multiplier;
    }

    const TextureBufferObject& RendererBackend::queryGbuffer(
        const gbuffer type, const bool gammaCorrect, const glm::vec4& defaultValue)
    {
        return mDebugPass.queryGBuffer(window::bufferSize(), mContext, type, gammaCorrect, defaultValue);
    }

    const TextureBufferObject& RendererBackend::tileOverlay()
    {
        return mDebugPass.tileOverlay(window::bufferSize(), mContext);
    }

    const TextureBufferObject& RendererBackend::getDebugBuffer()
    {
        return mDebugPass.getDebugOverlay();
    }

    const TextureBufferObject& RendererBackend::whtieFurnacetest()
    {
        return mDebugPass.whiteFurnaceTest(window::bufferSize(), mContext, mPrecalcs);
    }

    void RendererBackend::setUseUberVariant(const bool useUber)
    {
        mTileClassification.setUseUberVariant(useUber);
    }

    void RendererBackend::setupCurrentCamera(const CameraSettings& camera)
    {
        const float exposure = 1.f / (1.2f * glm::pow(2.f, camera.eV100));
        const glm::mat4 cameraProjectionMatrix = glm::perspective(camera.fovY, window::aspectRatio(), camera.nearClipDistance, camera.farClipDistance);
        const glm::mat4 vpMatrix = cameraProjectionMatrix * camera.viewMatrix;
        mContext.cameraViewProjectionMatrix = vpMatrix;

        mContext.camera->viewMatrix = camera.viewMatrix;
        mContext.camera->inverseVpMatrix = glm::inverse(vpMatrix);
        mContext.camera->position = glm::vec3(glm::inverse(camera.viewMatrix) * glm::vec4(glm::vec3(0.f), 1.f));
        mContext.camera->exposure = exposure;
        mContext.camera->zNear = camera.nearClipDistance;
        mContext.camera->zFar = camera.farClipDistance;
        mContext.camera.updateGlsl();
    }

    void RendererBackend::executePostProcessStack(const CameraSettings &camera)
    {
        PROFILE_FUNC();
        pushDebugGroup("Post-processing Pass");

        mContext.auxilliaryBuffer.resize(window::bufferSize());

        for (const std::unique_ptr<PostProcessLayer>  &postProcessLayer : camera.postProcessStack)
        {
            postProcessLayer->draw(&mContext.backBuffer, &mContext.auxilliaryBuffer, &mContext);
            copyTexture2D(mContext.auxilliaryBuffer, mContext.backBuffer); // Yes this is bad. I'm lazy.
        }

        popDebugGroup();
    }
}
