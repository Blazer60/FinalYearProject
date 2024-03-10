/**
 * @file RendererBackend.h
 * @author Ryan Purse
 * @date 09/03/2024
 */


#pragma once

#include "Context.h"
#include "GraphicsLighting.h"
#include "LightShading.h"
#include "MaterialRendering.h"
#include "Pch.h"
#include "ShadowMapping.h"
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
        RendererBackend() = default;
        void copyQueues(Queues &&queues);
        void execute();

    protected:
        void setupCurrentCamera(const CameraSettings &camera);

        Context mContext;
        MaterialRendering mMaterialRendering;
        TileClassification mTileClassification;
        ShadowMapping mShadowMapping;
        LightShading mLightShading;

        std::vector<RenderQueueObject> mRenderQueue;
        std::vector<CameraSettings> mCameraQueue;
        std::vector<DirectionalLight> mDirectionalLightQueue;
        std::vector<PointLight> mPointLightQueue;
        std::vector<Spotlight> mSpotlightQueue;
    };
} // graphics
