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
        void execute(const glm::ivec2 &size, Context &context, const std::vector<DirectionalLight> &directionalLightQueue);
        void execute(const glm::ivec2 &size, Context &context, const std::vector<PointLight> &pointLightQueue);
        void execute(const glm::ivec2 &size, Context &context, const std::vector<Spotlight> &spotLightQueue);
        void execute(const glm::ivec2 &size, Context &context, const Skybox &skybox);
    protected:
        void generateSpecularDirectionalAlbedoLut(const glm::ivec2 &size);
        void generateSpecularDirectionalAlbedoAverageLut(uint32_t size);
        void generateSpecularMissingLut(const glm::ivec2 &size);
        void setupLtcSheenTable();
        void generateSheenLut(const glm::ivec2 &size);
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

        Shader mIntegrateBrdfShader {
            { file::shaderPath() / "brdf/GgxDirectionalAlbedo.comp" }
        };

        Shader mIntegrateBrdfAverageShader {
            { file::shaderPath() / "brdf/GgxDirectionalAlbedoAverage.comp" }
        };

        Shader mIntegrateSpecularMissing {
            { file::shaderPath() / "brdf/GgxSpecMissing.comp" }
        };

        Shader mIntegrateSheenShader {
            { file::shaderPath() / "brdf/SheenDirectionalAlbedo.comp" }
        };

        std::vector<Shader> mIblShaderVariants;

        UniformBufferObject<DirectionalLightBlock> mDirectionalLightBlock;
        UniformBufferObject<PointLightBlock> mPointLightBlock;
        UniformBufferObject<SpotlightBlock> mSpotlightBlock;

        TextureBufferObject mSpecularDirectionalAlbedoLut = TextureBufferObject(textureFormat::Rg16f);
        TextureBufferObject mSpecularDirectionalAlbedoAverageLut = TextureBufferObject(textureFormat::Rg16f);
        TextureBufferObject mSpecularMissingTextureBuffer = TextureBufferObject(textureFormat::R16f);
        TextureBufferObject mLtcSheenTable = TextureBufferObject(textureFormat::Rgba16f);
        TextureBufferObject mSheenDirectionalAlbedoLut = TextureBufferObject(textureFormat::R16f);

        bool mUseUberVariant = false;
    };
} // graphics
