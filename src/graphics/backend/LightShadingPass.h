/**
 * @file LightShading.h
 * @author Ryan Purse
 * @date 10/03/2024
 */


#pragma once

#include "Context.h"
#include "Cubemap.h"
#include "DirectionalLightBlock.h"
#include "FileLoader.h"
#include "GraphicsLighting.h"
#include "HdrTexture.h"
#include "LookUpTables.h"
#include "Pch.h"
#include "PointLightBlock.h"
#include "Shader.h"
#include "SpotlightBlock.h"
#include "../Primitives.h"
#include "../Skybox.h"

namespace graphics
{
    /**
     * @author Ryan Purse
     * @date 10/03/2024
     */
    class LightShadingPass
    {
    public:
        LightShadingPass();
        void execute(const glm::ivec2 &size, Context &context, const Lut &lut, const std::vector<DirectionalLight> &directionalLightQueue);
        void execute(const glm::ivec2 &size, Context &context, const Lut &lut, const std::vector<PointLight> &pointLightQueue);
        void execute(const glm::ivec2 &size, Context &context, const Lut &lut, const std::vector<Spotlight> &spotLightQueue);
        void execute(const glm::ivec2 &size, Context &context, const Lut &lut, const Skybox &skybox);
    protected:
        void generateIblShaderVariants(const std::filesystem::path &path);

        Shader mDirectionalLightShader {
            { file::shaderPath() / "lighting/DirectionalLight.comp" }
        };

        Shader mPointLightShader {
            { file::shaderPath() / "lighting/PointLight.comp" }
        };

        Shader mSpotlightShader {
            { file::shaderPath() / "lighting/SpotLight.comp" }
        };

        Shader mIblShader {
            { file::shaderPath() / "lighting/IBL.comp" },
            { { "COMPUTE_SHEEN", 1 } }
        };

        std::vector<Shader> mIblShaderVariants;

        UniformBufferObject<DirectionalLightBlock> mDirectionalLightBlock;
        UniformBufferObject<PointLightBlock> mPointLightBlock;
        UniformBufferObject<SpotlightBlock> mSpotlightBlock;

        bool mUseUberVariant = false;
    };
} // graphics
