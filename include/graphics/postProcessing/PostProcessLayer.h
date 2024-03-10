/**
 * @file PostProcessLayer.h
 * @author Ryan Purse
 * @date 13/07/2023
 */


#pragma once

#include "FramebufferObject.h"

namespace graphics
{
    struct Context;
}

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
    
    void draw(TextureBufferObject *imageInput, TextureBufferObject *imageOutput, graphics::Context *context);

protected:
    virtual void onDraw(TextureBufferObject *imageInput, TextureBufferObject *imageOutput, graphics::Context *context) = 0;
    
    std::unique_ptr<FramebufferObject> mFramebuffer;
};
