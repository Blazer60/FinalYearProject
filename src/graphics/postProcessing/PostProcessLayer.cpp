/**
 * @file PostProcessLayer.cpp
 * @author Ryan Purse
 * @date 13/07/2023
 */


#include "PostProcessLayer.h"
#include "FramebufferObject.h"
#include "../backend/Context.h"

PostProcessLayer::PostProcessLayer()
    : mFramebuffer(std::make_unique<FramebufferObject>(GL_ONE, GL_ZERO, GL_ALWAYS))
{
}

void PostProcessLayer::draw(TextureBufferObject *imageInput, TextureBufferObject *imageOutput, graphics::Context *context)
{
    onDraw(imageInput, imageOutput, context);
}
