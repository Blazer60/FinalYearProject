/**
 * @file RendererBackend.h
 * @author Ryan Purse
 * @date 09/03/2024
 */


#pragma once

#include "Context.h"
#include "DebugPass.h"
#include "GraphicsLighting.h"
#include "LightShading.h"
#include "MaterialRendering.h"
#include "Pch.h"
#include "ShadowMapping.h"
#include "SkyboxPass.h"
#include "TileClassification.h"

namespace graphics
{
    struct Queues
    {
        std::vector<RenderQueueObject> renderQueue;
        std::vector<CameraSettings> cameraQueue;
        std::vector<DirectionalLight> directionalLightQueue;
        std::vector<PointLight> pointLightQueue;
        std::vector<Spotlight> spotlightQueue;
        std::vector<DebugQueueObject> debugQueue;
        std::vector<LineQueueObject> lineQueue;
    };

    /**
     * @author Ryan Purse
     * @date 09/03/2024
     */
    class RendererBackend
    {
    public:
        RendererBackend();
        const Context &getContext();
        void generateSkybox(std::string_view path, const glm::ivec2 &size);
        void copyQueues(Queues &&queues);
        void execute();
        void setIblMultiplier(float multiplier);
        const TextureBufferObject& queryGbuffer(gbuffer type, bool gammaCorrect, const glm::vec4 &defaultValue);
        const TextureBufferObject& tileOverlay();
        const TextureBufferObject& getDebugBuffer();

    protected:
        void setupCurrentCamera(const CameraSettings &camera);
        void executePostProcessStack(const CameraSettings &camera);

        Skybox mSkybox;

        Context mContext;
        MaterialRendering mMaterialRendering;
        TileClassification mTileClassification;
        ShadowMapping mShadowMapping;
        LightShading mLightShading;
        SkyboxPass mSkyboxPass;
        DebugPass mDebugPass;

        std::vector<RenderQueueObject> mRenderQueue;
        std::vector<CameraSettings> mCameraQueue;
        std::vector<DirectionalLight> mDirectionalLightQueue;
        std::vector<PointLight> mPointLightQueue;
        std::vector<Spotlight> mSpotlightQueue;
        std::vector<DebugQueueObject> mDebugQueue;
        std::vector<LineQueueObject> mLineQueue;
    };
} // graphics
