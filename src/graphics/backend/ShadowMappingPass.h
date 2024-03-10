/**
 * @file ShadowMapping.h
 * @author Ryan Purse
 * @date 09/03/2024
 */


#pragma once

#include "CameraSettings.h"
#include "Context.h"
#include "FileLoader.h"
#include "GraphicsLighting.h"
#include "Pch.h"

namespace graphics
{

    /**
     * @author Ryan Purse
     * @date 09/03/2024
     */
    class ShadowMappingPass
    {
    public:
        void execute(const std::vector<RenderQueueObject> &renderQueue, std::vector<PointLight> &pointLightQueue);
        void execute(const std::vector<RenderQueueObject> &renderQueue, const std::vector<Spotlight> &spotlightQueue);

        void execute(
            const CameraSettings &camera,
            const std::vector<RenderQueueObject> &renderQueue,
            std::vector<DirectionalLight> &directionalLightQueue);

    protected:
        FramebufferObject mFramebuffer = FramebufferObject(GL_ONE, GL_ZERO, GL_LESS);

        Shader mDirectionalLightShadowShader {
            { file::shaderPath() / "shadow/Shadow.vert", file::shaderPath() / "shadow/Shadow.frag" }
        };

        Shader mPointLightShadowShader {
            { file::shaderPath() / "shadow/PointShadow.vert", file::shaderPath() / "shadow/PointShadow.frag" }
        };

        Shader mSpotlightShadowShader {
            { file::shaderPath() / "shadow/PointShadow.vert", file::shaderPath() / "shadow/PointShadow.frag" }
        };
    };

} // graphics
