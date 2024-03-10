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
    void RendererBackend::copyQueues(Queues &&queues)
    {
        mRenderQueue = std::move(queues.renderQueue);
        mCameraQueue = std::move(queues.cameraQueue);
        mDirectionalLightQueue = std::move(queues.directionalLightQueue);
        mPointLightQueue = std::move(queues.pointLightQueue);
        mSpotlightQueue = std::move(queues.spotlightQueue);
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

            // Reset the viewport back to the normal size once we've finished rendering all the shadows.
            glViewport(0, 0, window::bufferSize().x, window::bufferSize().y);  // Todo: Probs don't need this here now right?

            mContext.lightBuffer.resize(window::bufferSize());
            mContext.lightBuffer.clear();
            mLightShading.execute(window::bufferSize(), mContext, mDirectionalLightQueue);
            mLightShading.execute(window::bufferSize(), mContext, mPointLightQueue);
            mLightShading.execute(window::bufferSize(), mContext, mSpotlightQueue);
            mLightShading.executeIbl(window::bufferSize(), mContext);

        }

        popDebugGroup();
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
}
