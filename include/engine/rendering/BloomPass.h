/**
 * @file BloomPass.h
 * @author Ryan Purse
 * @date 13/07/2023
 */


#pragma once

#include <ostream>
#include "Pch.h"
#include "PostProcessLayer.h"
#include "Shader.h"
#include "TextureBufferObject.h"
#include "Drawable.h"


/**
 * @author Ryan Purse
 * @date 13/07/2023
 */
class BloomPass
    : public PostProcessLayer, public ui::Drawable
{
protected:
    void onDraw(TextureBufferObject *imageInput, TextureBufferObject *imageOutput) override;
    
    void onDrawUi() override;
    
    void generateAuxiliaryBuffers(const glm::ivec2 &size);
    
    Shader mPreFilter   { "../resources/shaders/FullscreenTriangle.vert", "../resources/shaders/postProcessing/bloom/BloomPreFilter.frag"   };
    Shader mDownSample  { "../resources/shaders/FullscreenTriangle.vert", "../resources/shaders/postProcessing/bloom/BloomDownSample.frag"  };
    Shader mUpSample    { "../resources/shaders/FullscreenTriangle.vert", "../resources/shaders/postProcessing/bloom/BloomUpSample.frag"    };
    Shader mComposite   { "../resources/shaders/FullscreenTriangle.vert", "../resources/shaders/postProcessing/bloom/BloomComposite.frag"   };
    
protected:
    std::unique_ptr<TextureBufferObject> mDownSampleTexture;
    std::unique_ptr<TextureBufferObject> mUpSampleTexture;
    
    const int mMipLevelCount { 8 };
    
    glm::ivec2 mCurrentSize { 0 };
    
    glm::vec3 mLightKeyThreshold { 1.f };
    glm::vec3 mLightMaxThreshold { 1.f };
    float mLightKeyIntensity { 5.f };
    float mLightMaxIntensity { 20.f };
    float mBloomScale { 1.f };
};
