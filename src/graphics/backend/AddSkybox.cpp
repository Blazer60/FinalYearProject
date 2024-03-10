/**
 * @file AddSkybox.cpp
 * @author Ryan Purse
 * @date 10/03/2024
 */


#include "AddSkybox.h"

#include "GraphicsFunctions.h"

namespace graphics
{
    void AddSkybox::execute(const glm::ivec2& size, Context& context)
    {
        PROFILE_FUNC();
        // pushDebugGroup("Skybox Pass");
        //
        // context.backBuffer.clear();
        //
        // mCombineLightingShader.bind();
        //
        // const glm::mat4 viewMatrixNoPosition = glm::mat4(glm::mat3(camera.viewMatrix));
        // const glm::mat4 inverseViewProjection = glm::inverse(cameraProjectionMatrix * viewMatrixNoPosition);
        //
        // mCombineLightingShader.set("u_irradiance_texture", mLightTextureBuffer->getId(), 0);
        // // todo: emissive buffer should go straight in irradiance texture.
        // // mDeferredLightShader.set("u_emissive_texture", mEmissiveTextureBuffer->getId(), 1);
        // mCombineLightingShader.set("depthBufferTexture", mDepthTextureBuffer->getId(), 2);
        // mCombineLightingShader.set("u_skybox_texture", mHdrSkybox->getId(), 3);
        // mCombineLightingShader.set("u_reflection_texture", mReflectionTextureBuffer->getId(), 4);
        //
        // mCombineLightingShader.set("u_inverse_vp_matrix", inverseViewProjection);
        // mCombineLightingShader.set("u_luminance_multiplier", mIblLuminanceMultiplier);
        // mCombineLightingShader.set("u_exposure", exposure);
        // mCombineLightingShader.image("lighting", mCombinedLightingTextureBuffer->getId(), mCombinedLightingTextureBuffer->getFormat(), 1, false, GL_READ_WRITE);
        //
        // const auto threadGroupSize = glm::ivec2(ceil(glm::vec2(size) / glm::vec2(16)));
        //
        // glDispatchCompute(threadGroupSize.x, threadGroupSize.y, 1);
        //
        popDebugGroup();
    }
} // graphics
