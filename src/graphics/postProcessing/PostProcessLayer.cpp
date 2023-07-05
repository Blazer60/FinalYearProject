/**
 * @file PostProcessLayer.cpp
 * @author Ryan Purse
 * @date 13/07/2023
 */


#include "PostProcessLayer.h"
#include "FramebufferObject.h"
#include "TextureBufferObject.h"

PostProcessLayer::PostProcessLayer()
    : mFramebuffer(std::make_unique<FramebufferObject>(GL_ONE, GL_ZERO, GL_ALWAYS))
{
}

void PostProcessLayer::draw(TextureBufferObject *imageInput, TextureBufferObject *imageOutput)
{
    onDraw(imageInput, imageOutput);
}
