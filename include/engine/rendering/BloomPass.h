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
#include <FileLoader.h>


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
    
    Shader mPreFilter   { { file::shaderPath() / "FullscreenTriangle.vert", file::shaderPath() / "postProcessing/bloom/BloomPreFilter.frag"   } };
    Shader mDownSample  { { file::shaderPath() / "FullscreenTriangle.vert", file::shaderPath() / "postProcessing/bloom/BloomDownSample.frag"  } };
    Shader mUpSample    { { file::shaderPath() / "FullscreenTriangle.vert", file::shaderPath() / "postProcessing/bloom/BloomUpSample.frag"    } };
    Shader mComposite   { { file::shaderPath() / "FullscreenTriangle.vert", file::shaderPath() / "postProcessing/bloom/BloomComposite.frag"   } };
    
    std::unique_ptr<TextureBufferObject> mDownSampleTexture;
    std::unique_ptr<TextureBufferObject> mUpSampleTexture;
    
    int mMipLevelCount { 8 };
    const int mMaxMipLevelCount { 8 };
    
    glm::ivec2 mCurrentSize { 0 };
    
    glm::vec3 mColour { 1.f };
    float mBloomScale { 1.f };
    float mBloomStrength { 0.04f };
};
