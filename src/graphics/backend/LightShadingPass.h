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
    struct IblShaderVariant
    {
        Shader shader;
        std::function<void(Shader &, Context &, const Lut &, const Skybox &)> callback;
    };

    struct LightShaderVariant
    {
        Shader shader;
        std::function<void(Shader &, Context &, const Lut &)> callback;
    };

    /**
     * @author Ryan Purse
     * @date 10/03/2024
     */
    class LightShadingPass
    {
    public:
        LightShadingPass();
        void execute(Context &context, const Lut &lut, const std::vector<DirectionalLight> &directionalLightQueue);
        void execute(Context &context, const Lut &lut, const std::vector<PointLight> &pointLightQueue);
        void execute(Context&context, const Lut&lut, const std::vector<Spotlight>&spotLightQueue);
        void execute(Context &context, const Lut &lut, const Skybox &skybox);
    protected:
        void generateIblShaderVariants(const std::filesystem::path &path);
        std::vector<LightShaderVariant> generateLightShaderVariants(const std::filesystem::path &path);

        std::vector<IblShaderVariant> mIblShaderVariants;
        std::vector<LightShaderVariant> mDirectionalLightShaderVariants;
        std::vector<LightShaderVariant> mPointLightShaderVariants;
        std::vector<LightShaderVariant> mSpotlightShaderVariants;

        UniformBufferObject<DirectionalLightBlock> mDirectionalLightBlock;
        UniformBufferObject<PointLightBlock> mPointLightBlock;
        UniformBufferObject<SpotlightBlock> mSpotlightBlock;
    };
} // graphics
