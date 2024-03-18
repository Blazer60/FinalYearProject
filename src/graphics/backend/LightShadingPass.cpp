/**
 * @file LightShading.cpp
 * @author Ryan Purse
 * @date 10/03/2024
 */


#include "LightShadingPass.h"

#include "Cubemap.h"
#include "GraphicsFunctions.h"
#include "LtcSheenTable.h"

namespace graphics
{
    LightShadingPass::LightShadingPass()
    {
        generateIblShaderVariants(file::shaderPath() / "lighting/IBL.comp");
        for (auto &[ibl, _] : mIblShaderVariants)
            ibl.block("CameraBlock", 0);

        mDirectionalLightShaderVariants = generateLightShaderVariants(file::shaderPath() / "lighting/DirectionalLight.comp");
        for (auto &[shader, _] : mDirectionalLightShaderVariants)
        {
            shader.block("CameraBlock", 0);
            shader.block("DirectionalLightBlock", 1);
        }

        mPointLightShaderVariants = generateLightShaderVariants(file::shaderPath() / "lighting/PointLight.comp");
        for (auto &[shader, _] : mPointLightShaderVariants)
        {
           shader.block("CameraBlock", 0);
           shader.block("PointLightBlock", 1);
        }

        mSpotlightShaderVariants = generateLightShaderVariants(file::shaderPath() / "lighting/Spotlight.comp");
        for (auto &[shader, _] : mSpotlightShaderVariants)
        {
           shader.block("CameraBlock", 0);
           shader.block("SpotlightBlock", 1);
        }
    }

    void LightShadingPass::execute(Context& context, const Lut &lut, const std::vector<DirectionalLight>& directionalLightQueue)
    {
        if (directionalLightQueue.empty())
            return;

        PROFILE_FUNC();
        pushDebugGroup("Directional Lighting");

        context.camera.bindToSlot(0);
        mDirectionalLightBlock.bindToSlot(1);
        context.tileClassificationStorage.bindToSlot(1);

        int indirectOffset = 0;
        for (auto &[shader, callback] : mDirectionalLightShaderVariants)
        {
            shader.bind();

            shader.set("depthBufferTexture", context.depthBuffer.getId(), 0);
            shader.set("directionalAlbedoLut", lut.specularDirectionalAlbedo.getId(), 3);
            shader.set("directionalAlbedoAverageLut", lut.specularDirectionalAlbedoAverage.getId(), 4);
            shader.image("storageGBuffer", context.gbuffer.getId(), context.gbuffer.getFormat(), 0, true, GL_READ_ONLY);
            shader.image("lighting", context.lightBuffer.getId(), context.lightBuffer.getFormat(), 1, false, GL_READ_WRITE);

            callback(shader, context, lut);

            for (const DirectionalLight &directionalLight : directionalLightQueue)
            {
                mDirectionalLightBlock->direction = glm::vec4(directionalLight.direction, 0.f);
                mDirectionalLightBlock->intensity = glm::vec4(directionalLight.colourIntensity, 0.f);
                std::copy(directionalLight.vpMatrices.begin(), directionalLight.vpMatrices.end(), std::begin(mDirectionalLightBlock->vpMatrices));
                std::copy(directionalLight.cascadeDepths.begin(), directionalLight.cascadeDepths.end(), std::begin(mDirectionalLightBlock->cascadeDistances));
                mDirectionalLightBlock->bias = directionalLight.shadowBias;
                mDirectionalLightBlock->cascadeCount = static_cast<int>(directionalLight.cascadeDepths.size());

                mDirectionalLightBlock.updateGlsl();
                shader.set("u_shadow_map_texture", directionalLight.shadowMap->getId(), 1);

                dispatchComputeIndirect(context.tileClassificationStorage.getId(), indirectOffset * 4 * sizeof(uint32_t));
            }
            ++indirectOffset;
        }

        popDebugGroup();
    }

    void LightShadingPass::execute(Context& context, const Lut &lut, const std::vector<PointLight>& pointLightQueue)
    {
        if (pointLightQueue.empty())
            return;

        PROFILE_FUNC();
        pushDebugGroup("Point Lighting");

        context.camera.bindToSlot(0);
        mPointLightBlock.bindToSlot(1);
        context.tileClassificationStorage.bindToSlot(1);

        int indirectOffset = 0;
        for (auto &[shader, callback] : mPointLightShaderVariants)
        {
            shader.bind();
            shader.set("depthBufferTexture", context.depthBuffer.getId(), 0);
            shader.set("directionalAlbedoLut", lut.specularDirectionalAlbedo.getId(), 3);
            shader.set("directionalAlbedoAverageLut", lut.specularDirectionalAlbedoAverage.getId(), 4);
            shader.image("storageGBuffer", context.gbuffer.getId(), context.gbuffer.getFormat(), 0, true, GL_READ_ONLY);
            shader.image("lighting", context.lightBuffer.getId(), context.lightBuffer.getFormat(), 1, false, GL_READ_WRITE);
            callback(shader, context, lut);

            for (const auto &pointLight : pointLightQueue)
            {
                mPointLightBlock->position = glm::vec4(pointLight.position, 1.f);
                mPointLightBlock->intensity = glm::vec4(pointLight.colourIntensity, 0.f);
                mPointLightBlock->invSqrRadius = 1.f / (pointLight.radius * pointLight.radius);
                mPointLightBlock->zFar = pointLight.radius;
                mPointLightBlock->softnessRadius = pointLight.softnessRadius;
                mPointLightBlock->bias = pointLight.bias;
                const glm::mat4 pointLightModelMatrix = glm::translate(glm::mat4(1.f), pointLight.position) * glm::scale(glm::mat4(1.f), glm::vec3(pointLight.radius));
                mPointLightBlock->mvpMatrix = context.cameraViewProjectionMatrix * pointLightModelMatrix;
                mPointLightBlock.updateGlsl();

                shader.set("u_shadow_map_texture", pointLight.shadowMap->getId(), 1);

                dispatchComputeIndirect(context.tileClassificationStorage.getId(), indirectOffset * 4 * sizeof(uint32_t));
            }
            ++indirectOffset;
        }

        popDebugGroup();
    }

    void LightShadingPass::execute(Context&context, const Lut&lut, const std::vector<Spotlight>&spotLightQueue)
    {
        if (spotLightQueue.empty())
            return;

        PROFILE_FUNC();
        pushDebugGroup("Spot lighting");

        context.camera.bindToSlot(0);
        mSpotlightBlock.bindToSlot(1);
        context.tileClassificationStorage.bindToSlot(1);

        int indirectOffset = 0;
        for (auto &[shader, callback] : mSpotlightShaderVariants)
        {
            shader.bind();
            shader.set("depthBufferTexture", context.depthBuffer.getId(), 0);
            shader.set("directionalAlbedoLut", lut.specularDirectionalAlbedo.getId(), 3);
            shader.set("directionalAlbedoAverageLut", lut.specularDirectionalAlbedoAverage.getId(), 4);
            shader.image("storageGBuffer", context.gbuffer.getId(), context.gbuffer.getFormat(), 0, true, GL_READ_ONLY);
            shader.image("lighting", context.lightBuffer.getId(), context.lightBuffer.getFormat(), 1, false, GL_READ_WRITE);
            callback(shader, context, lut);

            for (const Spotlight &spotLight : spotLightQueue)
            {
                shader.set("u_shadow_map_texture", spotLight.shadowMap->getId(), 1);

                mSpotlightBlock->position = glm::vec4(spotLight.position, 1.f);
                mSpotlightBlock->direction = glm::vec4(spotLight.direction, 0.f);
                mSpotlightBlock->invSqrRadius = 1.f / (spotLight.radius * spotLight.radius);
                mSpotlightBlock->intensity = glm::vec4(spotLight.colourIntensity, 0.f);
                mSpotlightBlock->bias = spotLight.shadowBias;
                mSpotlightBlock->zFar = spotLight.radius;
                mSpotlightBlock->vpMatrix = spotLight.vpMatrix;
                const float lightAngleScale = 1.f / glm::max(0.001f, (spotLight.cosInnerAngle - spotLight.cosOuterAngle));
                const float lightAngleOffset = -spotLight.cosOuterAngle * lightAngleScale;
                mSpotlightBlock->angleScale = lightAngleScale;
                mSpotlightBlock->angleOffset = lightAngleOffset;

                mSpotlightBlock.updateGlsl();

                dispatchComputeIndirect(context.tileClassificationStorage.getId(), indirectOffset * 4 * sizeof(uint32_t));
            }
            ++indirectOffset;
        }

        popDebugGroup();
    }

    void LightShadingPass::execute(Context &context, const Lut &lut, const Skybox &skybox)
    {
        PROFILE_FUNC();
        if (!skybox.isValid)
            return;

        pushDebugGroup("Distant Light Probe");

        context.camera.bindToSlot(0);
        context.tileClassificationStorage.bindToSlot(1);

        for (auto &[shader, callback] : mIblShaderVariants)
            callback(shader, context, lut, skybox);

        popDebugGroup();
    }

    void LightShadingPass::generateIblShaderVariants(const std::filesystem::path &path)
    {
        const std::vector<Definition> uberShaderDefinitions {
            { "TILED_RENDERING", 1 },
            { "SHADER_INDEX", 0 },
            { "COMPUTE_SHEEN", 1 }
        };

        mIblShaderVariants.push_back( IblShaderVariant { Shader({ path }, uberShaderDefinitions),
            [](Shader &shader, const Context &context, const Lut &lut, const Skybox &skybox)
            {
                shader.image("storageGBuffer", context.gbuffer.getId(), context.gbuffer.getFormat(), 0, true, GL_READ_ONLY);
                shader.image("lighting", context.lightBuffer.getId(), context.lightBuffer.getFormat(), 1, false, GL_READ_WRITE);
                shader.set("depthBufferTexture", context.depthBuffer.getId(), 0);
                shader.set("missingSpecularLutTexture", lut.specularMissing.getId(), 1);
                shader.set("directionalAlbedoLut", lut.specularDirectionalAlbedo.getId(), 2);
                shader.set("directionalAlbedoAverageLut", lut.specularDirectionalAlbedoAverage.getId(), 3);
                shader.set("u_irradiance_texture", skybox.irradianceMap.getId(), 4);
                shader.set("u_pre_filter_texture", skybox.prefilterMap.getId(), 5);

                shader.set("sheenLut", lut.sheenDirectionalAlbedo.getId(), 6);
                shader.set("sheenMissing", lut.sheenMissing.getId(), 7);

                shader.set("u_luminance_multiplier", skybox.luminanceMultiplier);

                shader.bind();

                dispatchComputeIndirect(context.tileClassificationStorage.getId(), 4 * sizeof(uint32_t) * 0);
            }
        });

        const std::vector<Definition> baseShaderDefinitions {
            { "TILED_RENDERING", 1 },
            { "SHADER_INDEX", 1 },
            { "COMPUTE_SHEEN", 0 }
        };

        mIblShaderVariants.push_back(IblShaderVariant { Shader({ path }, baseShaderDefinitions),
        [](Shader &shader, const Context &context, const Lut &lut, const Skybox &skybox)
            {
                shader.image("storageGBuffer", context.gbuffer.getId(), context.gbuffer.getFormat(), 0, true, GL_READ_ONLY);
                shader.image("lighting", context.lightBuffer.getId(), context.lightBuffer.getFormat(), 1, false, GL_READ_WRITE);
                shader.set("depthBufferTexture", context.depthBuffer.getId(), 0);
                shader.set("missingSpecularLutTexture", lut.specularMissing.getId(), 1);
                shader.set("directionalAlbedoLut", lut.specularDirectionalAlbedo.getId(), 2);
                shader.set("directionalAlbedoAverageLut", lut.specularDirectionalAlbedoAverage.getId(), 3);
                shader.set("u_irradiance_texture", skybox.irradianceMap.getId(), 4);
                shader.set("u_pre_filter_texture", skybox.prefilterMap.getId(), 5);

                shader.set("u_luminance_multiplier", skybox.luminanceMultiplier);

                shader.bind();

                dispatchComputeIndirect(context.tileClassificationStorage.getId(), 4 * sizeof(uint32_t) * 1);
            }
        });
    }

    std::vector<LightShaderVariant> LightShadingPass::generateLightShaderVariants(const std::filesystem::path &path)
    {
        std::vector<LightShaderVariant> results;

        const std::vector<Definition> uberShaderDefinitions {
            { "SHADER_INDEX", 0 },
            { "COMPUTE_SHEEN", 1 }
        };

        results.push_back(LightShaderVariant { Shader( { path }, uberShaderDefinitions),
            [](Shader &shader, Context &context, const Lut &lut)
            {
                // Todo: Bindings here do not match the shader code.
                shader.set("sheenTable", lut.ltcSheenTable.getId(), 5);
                shader.set("sheenAlbedoLut", lut.sheenDirectionalAlbedo.getId(), 6);
            }
        });

        const std::vector<Definition> baseShaderDefinitions {
            { "SHADER_INDEX", 1 },
            { "COMPUTE_SHEEN", 0 }
        };

        results.push_back(LightShaderVariant { Shader( { path }, baseShaderDefinitions),
            [](Shader &shader, Context &context, const Lut &lut)
            {

            }
        });

        return results;
    }
}
