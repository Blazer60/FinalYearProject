/**
 * @file PostProcessLayer.h
 * @author Ryan Purse
 * @date 13/07/2023
 */


#pragma once

#include "Pch.h"
#include "FramebufferObject.h"

class TextureBufferObject;

/**
 * @author Ryan Purse
 * @date 13/07/2023
 */
class PostProcessLayer
{
public:
    PostProcessLayer();
    virtual ~PostProcessLayer() = default;
    
    void draw(TextureBufferObject *imageInput, TextureBufferObject *imageOutput);

protected:
    virtual void onDraw(TextureBufferObject *imageInput, TextureBufferObject *imageOutput) = 0;
    
    std::unique_ptr<FramebufferObject> mFramebuffer;
};
