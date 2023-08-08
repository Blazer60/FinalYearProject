/**
 * @file BloomPass.cpp
 * @author Ryan Purse
 * @date 13/07/2023
 */


#include "BloomPass.h"
#include "imgui.h"
#include "gtc/type_ptr.hpp"
#include "GraphicsState.h"

void BloomPass::onDraw(TextureBufferObject *imageInput, TextureBufferObject *imageOutput)
{
    if (mCurrentSize != imageOutput->getSize())
        generateAuxiliaryBuffers(imageOutput->getSize());
    
    mFramebuffer->bind();
    
    // pre-filter pass.
    const glm::ivec2 downSampleSize = mDownSampleTexture->getSize();
    glViewport(0, 0, downSampleSize.x, downSampleSize.y);
    mFramebuffer->attach(mDownSampleTexture.get(), 0, 0);
    mPreFilter.bind();
    mPreFilter.set("u_texture", imageInput->getId(), 0);
    mPreFilter.set("u_light_key_threshold", mLightKeyThreshold * mLightKeyIntensity);
    mPreFilter.set("u_light_max_threshold", mLightMaxThreshold * mLightMaxIntensity);
    graphics::renderer->drawFullscreenTriangleNow();
    mFramebuffer->detach(0);
    
    // down-sample pass.
    mDownSample.bind();
    mDownSample.set("u_texture", mDownSampleTexture->getId(), 0);
    for (int i = 1; i < mMipLevelCount; ++i)
    {
        glViewport(0, 0, downSampleSize.x >> i, downSampleSize.y >> i);
        mDownSample.set("u_mip_level", i - 1);
        mFramebuffer->attach(mDownSampleTexture.get(), 0, i);
        graphics::renderer->drawFullscreenTriangleNow();
        mFramebuffer->detach(0);
    }
    
    // up-sample pass.
    const glm::ivec2 upSampleSize = mUpSampleTexture->getSize();
    mUpSample.bind();
    const auto count = mMipLevelCount;
    mUpSample.set("u_scale", mBloomScale);
    
    // first pass uses the two smallest mips from the down-sample.
    glViewport(0, 0, upSampleSize.x >> count, upSampleSize.y >> count);
    mUpSample.set("u_up_sample_texture", mDownSampleTexture->getId(), 0);
    mUpSample.set("u_down_sample_texture", mDownSampleTexture->getId(), 1);
    mUpSample.set("u_up_mip_level", count - 1);
    mUpSample.set("u_down_mip_level", count - 2);
    
    mFramebuffer->attach(mUpSampleTexture.get(), 0, count - 1);
    graphics::renderer->drawFullscreenTriangleNow();
    mFramebuffer->detach(0);
    
    mUpSample.set("u_up_sample_texture", mUpSampleTexture->getId(), 0);  // Up-sample texture changes to the correct one.
    for (int i = count - 2; i > 0; --i)
    {
        glViewport(0, 0, upSampleSize.x >> i, upSampleSize.y >> i);
        mUpSample.set("u_up_mip_level", i + 1);
        mUpSample.set("u_down_mip_level", i - 1);
        
        mFramebuffer->attach(mUpSampleTexture.get(), 0, i);
        graphics::renderer->drawFullscreenTriangleNow();
        mFramebuffer->detach(0);
    }
    
    // composite stage.
    glViewport(0, 0, imageOutput->getSize().x, imageOutput->getSize().y);
    mComposite.bind();
    mComposite.set("u_original", imageInput->getId(), 0);
    mComposite.set("u_bloom", mUpSampleTexture->getId(), 1);
    
    mFramebuffer->attach(imageOutput, 0);
    graphics::renderer->drawFullscreenTriangleNow();
    mFramebuffer->detach(0);
}

void BloomPass::generateAuxiliaryBuffers(const glm::ivec2 &size)
{
    mMipLevelCount = glm::min(mMaxMipLevelCount, (int)glm::floor(glm::log2((float)glm::max(size.x, size.y))));
    mDownSampleTexture  = std::make_unique<TextureBufferObject>(size / 2, GL_RGBA16F, GL_LINEAR_MIPMAP_NEAREST, GL_LINEAR, mMipLevelCount);
    mUpSampleTexture    = std::make_unique<TextureBufferObject>(size    , GL_RGBA16F, GL_LINEAR_MIPMAP_NEAREST, GL_LINEAR, mMipLevelCount);
}

void BloomPass::onDrawUi()
{
    if (ImGui::TreeNode("Bloom"))
    {
        ImGui::ColorEdit3("Light key threshold", glm::value_ptr(mLightKeyThreshold), ImGuiColorEditFlags_HDR | ImGuiColorEditFlags_Float | ImGuiColorEditFlags_PickerHueWheel);
        ImGui::DragFloat("Intensity threshold", &mLightKeyIntensity, 0.001f);
        
        ImGui::ColorEdit3("Light Max threshold", glm::value_ptr(mLightMaxThreshold), ImGuiColorEditFlags_HDR | ImGuiColorEditFlags_Float | ImGuiColorEditFlags_PickerHueWheel);
        ImGui::DragFloat("Intensity Max", &mLightMaxIntensity, 0.001f);
        
        ImGui::DragFloat("Bloom Scale", &mBloomScale, 0.01f);
        ImGui::TreePop();
    }
}
