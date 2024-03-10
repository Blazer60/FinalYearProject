/**
 * @file SkyboxPass.cpp
 * @author Ryan Purse
 * @date 10/03/2024
 */


#include "SkyboxPass.h"

#include "GraphicsFunctions.h"
#include "../Skybox.h"

namespace graphics
{
    void SkyboxPass::execute(const glm::ivec2& size, Context& context, const Skybox &skybox)
    {
        PROFILE_FUNC();
        pushDebugGroup("Skybox Pass");

        context.backBuffer.resize(size);
        context.backBuffer.clear();

        mCombineLightingShader.bind();

        mCombineLightingShader.set("u_irradiance_texture", context.lightBuffer.getId(), 0);
        mCombineLightingShader.set("u_emissive_texture", 0u, 1);
        mCombineLightingShader.set("depthBufferTexture", context.depthBuffer.getId(), 2);
        mCombineLightingShader.set("u_skybox_texture", skybox.hdrSkybox.getId(), 3);
        mCombineLightingShader.set("u_reflection_texture", 0u, 4);
        mCombineLightingShader.set("u_inverse_vp_matrix", glm::inverse(context.cameraViewProjectionMatrix));
        mCombineLightingShader.set("u_luminance_multiplier", skybox.luminanceMultiplier);
        mCombineLightingShader.set("u_exposure", context.camera->exposure);
        mCombineLightingShader.image("lighting", context.backBuffer.getId(), context.backBuffer.getFormat(), 1, false, GL_READ_WRITE);

        const auto threadGroupSize = glm::ivec2(ceil(glm::vec2(size) / glm::vec2(16)));

        glDispatchCompute(threadGroupSize.x, threadGroupSize.y, 1);

        popDebugGroup();
    }
} // graphics
