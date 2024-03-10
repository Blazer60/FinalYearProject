/**
 * @file LightShading.cpp
 * @author Ryan Purse
 * @date 10/03/2024
 */


#include "LightShading.h"

#include "Cubemap.h"
#include "GraphicsFunctions.h"
#include "HdrTexture.h"
#include "LtcSheenTable.h"

namespace graphics
{
    LightShading::LightShading()
    {
        generateIblShaderVariants(file::shaderPath() / "lighting/IBL.comp");

        constexpr int lutSize = 32;
        generateSpecularDirectionalAlbedoLut(glm::ivec2(lutSize));
        generateSpecularDirectionalAlbedoAverageLut(lutSize);

        setupLtcSheenTable();
        generateSheenLut(glm::ivec2(lutSize));

        mDirectionalLightShader.block("CameraBlock", 0);
        mDirectionalLightShader.block("DirectionalLightBlock", 1);

        mPointLightShader.block("CameraBlock", 0);
        mPointLightShader.block("PointLightBlock", 1);

        mSpotlightShader.block("CameraBlock", 0);
        mSpotlightShader.block("SpotlightBlock", 1);

        mIblShader.block("CameraBlock", 0);
        for (auto &ibl : mIblShaderVariants)
            ibl.block("CameraBlock", 0);
    }

    void LightShading::execute(
        const glm::ivec2& size, Context& context, const std::vector<DirectionalLight>& directionalLightQueue)
    {
        PROFILE_FUNC();
        pushDebugGroup("Directional Lighting");

        mDirectionalLightShader.bind();
        context.camera.bindToSlot(0);
        mDirectionalLightBlock.bindToSlot(1);

        mDirectionalLightShader.set("depthBufferTexture", context.depthBuffer.getId(), 0);
        mDirectionalLightShader.set("directionalAlbedoLut", mSpecularDirectionalAlbedoLut.getId(), 3);
        mDirectionalLightShader.set("directionalAlbedoAverageLut", mSpecularDirectionalAlbedoAverageLut.getId(), 4);
        mDirectionalLightShader.set("sheenTable", mLtcSheenTable.getId(), 5);
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

    void LightShading::execute(const glm::ivec2& size, Context& context, const std::vector<PointLight>& pointLightQueue)
    {
        PROFILE_FUNC();
        pushDebugGroup("Point Lighting");

        mPointLightShader.bind();
        context.camera.bindToSlot(0);
        mSpotlightBlock.bindToSlot(1);

        mPointLightShader.set("depthBufferTexture", context.depthBuffer.getId(), 0);
        mPointLightShader.set("directionalAlbedoLut", mSpecularDirectionalAlbedoLut.getId(), 3);
        mPointLightShader.set("directionalAlbedoAverageLut", mSpecularDirectionalAlbedoAverageLut.getId(), 4);
        mPointLightShader.set("sheenTable", mLtcSheenTable.getId(), 5);
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

    void LightShading::execute(const glm::ivec2& size, Context& context, const std::vector<Spotlight>& spotLightQueue)
    {
        PROFILE_FUNC();
        pushDebugGroup("Spot lighting");

        mSpotlightShader.bind();
        context.camera.bindToSlot(0);
        mSpotlightBlock.bindToSlot(1);

        mSpotlightShader.set("depthBufferTexture", context.depthBuffer.getId(), 0);
        mSpotlightShader.set("directionalAlbedoLut", mSpecularDirectionalAlbedoLut.getId(), 3);
        mSpotlightShader.set("directionalAlbedoAverageLut", mSpecularDirectionalAlbedoAverageLut.getId(), 4);
        mSpotlightShader.set("sheenTable", mLtcSheenTable.getId(), 5);
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

    void LightShading::executeIbl(const glm::ivec2& size, Context& context)
    {
        PROFILE_FUNC();
        if (!mHasSkybox)
            return;

        pushDebugGroup("Distant Light Probe");

        context.camera.bindToSlot(0);

        if (mUseUberVariant)
        {
            // todo: Do I need this as a seperate shader or should I make the tile-classification force all tiles to be the uber shader variant?
            mIblShader.image("storageGBuffer", context.gbuffer.getId(), context.gbuffer.getFormat(), 0, true, GL_READ_ONLY);
            mIblShader.image("lighting", context.lightBuffer.getId(), context.lightBuffer.getFormat(), 1, false, GL_READ_WRITE);
            mIblShader.set("depthBufferTexture", context.depthBuffer.getId(), 0);
            mIblShader.set("missingSpecularLutTexture", mSpecularMissingTextureBuffer.getId(), 1);
            mIblShader.set("directionalAlbedoLut", mSpecularDirectionalAlbedoLut.getId(), 2);
            mIblShader.set("directionalAlbedoAverageLut", mSpecularDirectionalAlbedoAverageLut.getId(), 3);
            mIblShader.set("u_irradiance_texture", mIrradianceMap.getId(), 4);
            mIblShader.set("u_pre_filter_texture", mPrefilterMap.getId(), 5);
            mIblShader.set("sheenLut", mSheenDirectionalAlbedoLut.getId(), 6);
            mIblShader.set("u_luminance_multiplier", mIblLuminanceMultiplier);

            mIblShader.bind();
            const auto threadGroupSize = glm::ivec2(ceil(glm::vec2(size) / glm::vec2(16)));
            glDispatchCompute(threadGroupSize.x, threadGroupSize.y, 1);
        }
        else
        {
            int i = 0;
            for (auto &iblShader : mIblShaderVariants)
            {
                iblShader.image("storageGBuffer", context.gbuffer.getId(), context.gbuffer.getFormat(), 0, true, GL_READ_ONLY);
                iblShader.image("lighting", context.lightBuffer.getId(), context.lightBuffer.getFormat(), 1, false, GL_READ_WRITE);
                iblShader.set("depthBufferTexture", context.depthBuffer.getId(), 0);
                iblShader.set("missingSpecularLutTexture", mSpecularMissingTextureBuffer.getId(), 1);
                iblShader.set("directionalAlbedoLut", mSpecularDirectionalAlbedoLut.getId(), 2);
                iblShader.set("directionalAlbedoAverageLut", mSpecularDirectionalAlbedoAverageLut.getId(), 3);
                iblShader.set("u_irradiance_texture", mIrradianceMap.getId(), 4);
                iblShader.set("u_pre_filter_texture", mPrefilterMap.getId(), 5);
                if (i == 0)
                    iblShader.set("sheenLut", mSheenDirectionalAlbedoLut.getId(), 6);

                iblShader.set("u_luminance_multiplier", mIblLuminanceMultiplier);
                iblShader.set("shaderIndex", i);

                iblShader.bind();

                dispatchComputeIndirect(context.tileClassificationStorage.getId(), 4 * sizeof(uint32_t) * i);
                ++i;
            }
        }

        popDebugGroup();
    }

    void LightShading::generateSkybox(const std::string_view path, const glm::ivec2 size)
    {
        HdrTexture hdrImage(path);
        hdrImage.setDebugName("HDR Skybox Image");
        createCubemapFromHdrTexture(hdrImage, size);
        generateIrradianceMap(size / 8);
        generatePrefilterMap(size / 4);
        mHasSkybox = true;
    }

    void LightShading::generateSpecularDirectionalAlbedoLut(const glm::ivec2& size)
    {
        mSpecularDirectionalAlbedoLut.resize(size);
        mSpecularDirectionalAlbedoLut.setDebugName("Specular BRDF LUT");

        mIntegrateBrdfShader.bind();
        mIntegrateBrdfShader.image("lut",
            mSpecularDirectionalAlbedoLut.getId(), mSpecularDirectionalAlbedoLut.getFormat(),
            0, false);

        const glm::uvec2 groupSize = glm::ceil(static_cast<glm::vec2>(size / 8));
        glDispatchCompute(groupSize.x, groupSize.y, 1);
    }

    void LightShading::generateSpecularDirectionalAlbedoAverageLut(const uint32_t size)
    {
        mSpecularDirectionalAlbedoAverageLut.resize(glm::ivec2(size, 1));
        mSpecularDirectionalAlbedoAverageLut.setDebugName("BRDF Average LUT");

        mIntegrateBrdfAverageShader.bind();
        mIntegrateBrdfAverageShader.set("brdfLut", mSpecularDirectionalAlbedoLut.getId(), 0);
        mIntegrateBrdfAverageShader.image("brdfAverageLut",
            mSpecularDirectionalAlbedoAverageLut.getId(), mSpecularDirectionalAlbedoAverageLut.getFormat(),
            0, false, GL_WRITE_ONLY);

        const uint32_t groupSize = glm::ceil(size / 8);
        glDispatchCompute(groupSize, 1, 1);
    }

    void LightShading::generateSpecularMissingLut(const glm::ivec2& size)
    {
        mSpecularMissingTextureBuffer.resize(size);

        mIntegrateSpecularMissing.bind();
        mIntegrateSpecularMissing.set("directionalAlbedoLut", mSpecularDirectionalAlbedoLut.getId(), 0);
        mIntegrateSpecularMissing.set("directionalAlbedoAverageLut", mSpecularDirectionalAlbedoAverageLut.getId(), 1);
        mIntegrateSpecularMissing.image("specMissing", mSpecularMissingTextureBuffer.getId(), mSpecularMissingTextureBuffer.getFormat(), 0, false);
        const glm::uvec2 groupSize = glm::ceil(static_cast<glm::vec2>(size / 8));
        glDispatchCompute(groupSize.x, groupSize.y, 1);
    }

    void LightShading::setupLtcSheenTable()
    {
        mLtcSheenTable.resize(glm::ivec2(sheen::tableSize));
        mLtcSheenTable.setDebugName("LTC Sheen LUT");

        // todo: double check the missmatch in types here. It may not like it?
        const auto sheenData = sheen::data();
        mLtcSheenTable.upload(sheenData.data(), pixelFormat::Rgb);
    }

    void LightShading::generateSheenLut(const glm::ivec2& size)
    {
        mSheenDirectionalAlbedoLut.resize(size);
        mSheenDirectionalAlbedoLut.setDebugName("Sheen Directional Albedo");

        mIntegrateSheenShader.bind();
        mIntegrateSheenShader.set("sheenTable", mLtcSheenTable.getId(), 0);
        mIntegrateSheenShader.image("sheenDirectionalAlbedo", mSheenDirectionalAlbedoLut.getId(), mSheenDirectionalAlbedoLut.getFormat(), 0, false);

        const glm::uvec2 groupSize = glm::ceil(static_cast<glm::vec2>(size / 8));
        glDispatchCompute(groupSize.x, groupSize.y, 1);
    }

    void LightShading::generateIblShaderVariants(const std::filesystem::path &path)
    {
        const std::vector<graphics::Definition> uberShaderDefinitions {
            { "TILED_RENDERING", 1 },
            { "COMPUTE_SHEEN", 1 }
        };

        const std::vector<graphics::Definition> baseShaderDefinitions {
            { "TILED_RENDERING", 1 },
            { "COMPUTE_SHEEN", 0 }
        };

        mIblShaderVariants.push_back(Shader({ path }, uberShaderDefinitions));
        mIblShaderVariants.push_back(Shader({ path }, baseShaderDefinitions));
    }

    void LightShading::createCubemapFromHdrTexture(const HdrTexture& hdrImage, glm::ivec2 size)
    {
        mHdrSkybox.resize(size);

        FramebufferObject auxiliaryFrameBuffer(GL_ONE, GL_ONE, GL_ALWAYS);

        const glm::mat4 views[] = {
            glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3( 1.0f,  0.0f,  0.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
            glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(-1.0f,  0.0f,  0.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
            glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3( 0.0f, -1.0f,  0.0f), glm::vec3(0.0f,  0.0f, -1.0f)),
            glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3( 0.0f,  1.0f,  0.0f), glm::vec3(0.0f,  0.0f,  1.0f)),
            glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3( 0.0f,  0.0f,  1.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
            glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3( 0.0f,  0.0f, -1.0f), glm::vec3(0.0f, -1.0f,  0.0f))
        };

        auxiliaryFrameBuffer.bind();
        mHdrToCubemapShader.bind();
        mHdrToCubemapShader.set("u_texture", hdrImage.getId(), 0);

        glViewport(0, 0, size.x, size.y);

        const SubMesh fullscreenTriangle = primitives::fullscreenTriangle();
        glBindVertexArray(fullscreenTriangle.vao());

        for (int i = 0; i < 6; ++i)
        {
            mHdrToCubemapShader.set("u_view_matrix", views[i]);
            auxiliaryFrameBuffer.attach(&mHdrSkybox, 0, i);
            auxiliaryFrameBuffer.clear(glm::vec4(glm::vec3(0.f), 1.f));

            glDrawElements(GL_TRIANGLES, fullscreenTriangle.indicesCount(), GL_UNSIGNED_INT, nullptr);

            auxiliaryFrameBuffer.detach(0);
        }
    }

    void LightShading::generateIrradianceMap(const glm::ivec2 size)
    {
        mIrradianceMap.resize(size);

        FramebufferObject auxiliaryFrameBuffer(GL_ONE, GL_ONE, GL_ALWAYS);

        const glm::mat4 views[] = {
            glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3( 1.0f,  0.0f,  0.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
            glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(-1.0f,  0.0f,  0.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
            glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3( 0.0f, -1.0f,  0.0f), glm::vec3(0.0f,  0.0f, -1.0f)),
            glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3( 0.0f,  1.0f,  0.0f), glm::vec3(0.0f,  0.0f,  1.0f)),
            glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3( 0.0f,  0.0f,  1.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
            glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3( 0.0f,  0.0f, -1.0f), glm::vec3(0.0f, -1.0f,  0.0f))
        };

        auxiliaryFrameBuffer.bind();
        mCubemapToIrradianceShader.bind();
        mCubemapToIrradianceShader.set("u_environment_texture", mHdrSkybox.getId(), 0);

        glViewport(0, 0, size.x, size.y);

        const auto fullscreenTriangle = primitives::fullscreenTriangle();
        glBindVertexArray(fullscreenTriangle.vao());

        for (int i = 0; i < 6; ++i)
        {
            mCubemapToIrradianceShader.set("u_view_matrix", views[i]);
            auxiliaryFrameBuffer.attach(&mIrradianceMap, 0, i);
            auxiliaryFrameBuffer.clear(glm::vec4(glm::vec3(0.f), 1.f));

            glDrawElements(GL_TRIANGLES, fullscreenTriangle.indicesCount(), GL_UNSIGNED_INT, nullptr);

            auxiliaryFrameBuffer.detach(0);
        }
    }

    void LightShading::generatePrefilterMap(const glm::ivec2 size)
    {
        mPrefilterMap.resize(size);

        FramebufferObject auxiliaryFrameBuffer(GL_ONE, GL_ONE, GL_ALWAYS);

        const glm::mat4 views[] = {
            glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3( 1.0f,  0.0f,  0.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
            glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(-1.0f,  0.0f,  0.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
            glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3( 0.0f, -1.0f,  0.0f), glm::vec3(0.0f,  0.0f, -1.0f)),
            glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3( 0.0f,  1.0f,  0.0f), glm::vec3(0.0f,  0.0f,  1.0f)),
            glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3( 0.0f,  0.0f,  1.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
            glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3( 0.0f,  0.0f, -1.0f), glm::vec3(0.0f, -1.0f,  0.0f))
        };

        auxiliaryFrameBuffer.bind();
        mPreFilterShader.bind();
        mPreFilterShader.set("u_environment_texture", mHdrSkybox.getId(), 0);

        const auto fullscreenTriangle = primitives::fullscreenTriangle();
        glBindVertexArray(fullscreenTriangle.vao());

        for (int mip = 0; mip < mPrefilterMap.getMipLevels(); ++mip)
        {
            const glm::ivec2 mipSize = size >> mip;
            glViewport(0, 0, mipSize.x, mipSize.y);

            const float roughness = static_cast<float>(mip) / (static_cast<float>(mPrefilterMap.getMipLevels()) - 1.f);
            mPreFilterShader.set("u_roughness", roughness);

            for (int i = 0; i < 6; ++i)
            {
                mPreFilterShader.set("u_view_matrix", views[i]);
                auxiliaryFrameBuffer.attach(&mPrefilterMap, 0, i, mip);
                auxiliaryFrameBuffer.clear(glm::vec4(glm::vec3(0.f), 1.f));

                glDrawElements(GL_TRIANGLES, fullscreenTriangle.indicesCount(), GL_UNSIGNED_INT, nullptr);

                auxiliaryFrameBuffer.detach(0);
            }
        }
    }
}
