/**
 * @file LightShading.cpp
 * @author Ryan Purse
 * @date 10/03/2024
 */


#include "LightShadingPass.h"

#include "Cubemap.h"
#include "GBufferFlags.h"
#include "GraphicsFunctions.h"
#include "HdrTexture.h"
#include "LtcSheenTable.h"

namespace graphics
{
    LightShadingPass::LightShadingPass()
    {
        generateIblShaderVariants(file::shaderPath() / "lighting/IBL.comp");

        mDirectionalLightShader.block("CameraBlock", 0);
        mDirectionalLightShader.block("DirectionalLightBlock", 1);

        mPointLightShader.block("CameraBlock", 0);
        mPointLightShader.block("PointLightBlock", 1);

        mSpotlightShader.block("CameraBlock", 0);
        mSpotlightShader.block("SpotlightBlock", 1);

        for (auto &[ibl, _] : mIblShaderVariants)
            ibl.block("CameraBlock", 0);
    }

    void LightShadingPass::execute(
        const glm::ivec2& size, Context& context, const Lut &lut, const std::vector<DirectionalLight>& directionalLightQueue)
    {
        PROFILE_FUNC();
        pushDebugGroup("Directional Lighting");

        mDirectionalLightShader.bind();
        context.camera.bindToSlot(0);
        mDirectionalLightBlock.bindToSlot(1);

        mDirectionalLightShader.set("depthBufferTexture", context.depthBuffer.getId(), 0);
        mDirectionalLightShader.set("directionalAlbedoLut", lut.specularDirectionalAlbedo.getId(), 3);
        mDirectionalLightShader.set("directionalAlbedoAverageLut", lut.specularDirectionalAlbedoAverage.getId(), 4);
        mDirectionalLightShader.set("sheenTable", lut.ltcSheenTable.getId(), 5);
        mDirectionalLightShader.image("storageGBuffer", context.gbuffer.getId(), context.gbuffer.getFormat(), 0, true, GL_READ_ONLY);
        mDirectionalLightShader.image("lighting", context.lightBuffer.getId(), context.lightBuffer.getFormat(), 1, false, GL_READ_WRITE);

        const auto threadGroupSize = glm::ivec2(ceil(glm::vec2(size) / glm::vec2(16)));

        for (const DirectionalLight &directionalLight : directionalLightQueue)
        {
            mDirectionalLightBlock->direction = glm::vec4(directionalLight.direction, 0.f);
            mDirectionalLightBlock->intensity = glm::vec4(directionalLight.colourIntensity, 0.f);
            std::copy(directionalLight.vpMatrices.begin(), directionalLight.vpMatrices.end(), std::begin(mDirectionalLightBlock->vpMatrices));
            std::copy(directionalLight.cascadeDepths.begin(), directionalLight.cascadeDepths.end(), std::begin(mDirectionalLightBlock->cascadeDistances));
            mDirectionalLightBlock->bias = directionalLight.shadowBias;
            mDirectionalLightBlock->cascadeCount = static_cast<int>(directionalLight.cascadeDepths.size());

            mDirectionalLightBlock.updateGlsl();
            mDirectionalLightShader.set("u_shadow_map_texture", directionalLight.shadowMap->getId(), 1);

            glDispatchCompute(threadGroupSize.x, threadGroupSize.y, 1);
        }

        popDebugGroup();
    }

    void LightShadingPass::execute(const glm::ivec2& size, Context& context, const Lut &lut, const std::vector<PointLight>& pointLightQueue)
    {
        PROFILE_FUNC();
        pushDebugGroup("Point Lighting");

        mPointLightShader.bind();
        context.camera.bindToSlot(0);
        mPointLightBlock.bindToSlot(1);

        mPointLightShader.set("depthBufferTexture", context.depthBuffer.getId(), 0);
        mPointLightShader.set("directionalAlbedoLut", lut.specularDirectionalAlbedo.getId(), 3);
        mPointLightShader.set("directionalAlbedoAverageLut", lut.specularDirectionalAlbedoAverage.getId(), 4);
        mPointLightShader.set("sheenTable", lut.ltcSheenTable.getId(), 5);
        mPointLightShader.image("storageGBuffer", context.gbuffer.getId(), context.gbuffer.getFormat(), 0, true, GL_READ_ONLY);
        mPointLightShader.image("lighting", context.lightBuffer.getId(), context.lightBuffer.getFormat(), 1, false, GL_READ_WRITE);

        const auto threadGroupSize = glm::ivec2(ceil(glm::vec2(size) / glm::vec2(16)));

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

            mPointLightShader.set("u_shadow_map_texture", pointLight.shadowMap->getId(), 1);

            glDispatchCompute(threadGroupSize.x, threadGroupSize.y, 1);
        }

        popDebugGroup();
    }

    void LightShadingPass::execute(const glm::ivec2& size, Context& context, const Lut &lut, const std::vector<Spotlight>& spotLightQueue)
    {
        PROFILE_FUNC();
        pushDebugGroup("Spot lighting");

        mSpotlightShader.bind();
        context.camera.bindToSlot(0);
        mSpotlightBlock.bindToSlot(1);

        mSpotlightShader.set("depthBufferTexture", context.depthBuffer.getId(), 0);
        mSpotlightShader.set("directionalAlbedoLut", lut.specularDirectionalAlbedo.getId(), 3);
        mSpotlightShader.set("directionalAlbedoAverageLut", lut.specularDirectionalAlbedoAverage.getId(), 4);
        mSpotlightShader.set("sheenTable", lut.ltcSheenTable.getId(), 5);
        mSpotlightShader.image("storageGBuffer", context.gbuffer.getId(), context.gbuffer.getFormat(), 0, true, GL_READ_ONLY);
        mSpotlightShader.image("lighting", context.lightBuffer.getId(), context.lightBuffer.getFormat(), 1, false, GL_READ_WRITE);

        const auto threadGroupSize = glm::ivec2(ceil(glm::vec2(size) / glm::vec2(16)));

        for (const Spotlight &spotLight : spotLightQueue)
        {
            mSpotlightShader.set("u_shadow_map_texture", spotLight.shadowMap->getId(), 1);

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

            glDispatchCompute(threadGroupSize.x, threadGroupSize.y, 1);
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

        for (auto &[shader, callback] : mIblShaderVariants)
            callback(shader, context, lut, skybox);

        popDebugGroup();
    }

    void LightShadingPass::generateIblShaderVariants(const std::filesystem::path &path)
    {
        int shaderIndex = 0;
        const std::vector<Definition> uberShaderDefinitions {
            { "TILED_RENDERING", 1 },
            { "COMPUTE_SHEEN", 1 }
        };

        mIblShaderVariants.push_back( IblShaderVariant { Shader({ path }, uberShaderDefinitions),
            [shaderIndex](Shader &shader, const Context &context, const Lut &lut, const Skybox &skybox)
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

                shader.set("u_luminance_multiplier", skybox.luminanceMultiplier);
                shader.set("shaderIndex", shaderIndex);

                shader.bind();

                dispatchComputeIndirect(context.tileClassificationStorage.getId(), 4 * sizeof(uint32_t) * shaderIndex);
            }
        });
        ++shaderIndex;

        const std::vector<Definition> baseShaderDefinitions {
            { "TILED_RENDERING", 1 },
            { "COMPUTE_SHEEN", 0 }
        };

        mIblShaderVariants.push_back(IblShaderVariant { Shader({ path }, baseShaderDefinitions),
        [shaderIndex](Shader &shader, const Context &context, const Lut &lut, const Skybox &skybox)
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
                shader.set("shaderIndex", shaderIndex);

                shader.bind();

                dispatchComputeIndirect(context.tileClassificationStorage.getId(), 4 * sizeof(uint32_t) * shaderIndex);
            }
        });
        ++shaderIndex;
    }
}
