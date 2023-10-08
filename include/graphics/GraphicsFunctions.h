/**
 * @file RendererFunctions.h
 * @author Ryan Purse
 * @date 15/07/2023
 */


#pragma once

#include "Pch.h"

#include "TextureBufferObject.h"
#include "TextureArrayObject.h"

namespace graphics
{
    std::unique_ptr<TextureBufferObject> cloneTextureLayer(const TextureArrayObject &from, int layer);
    
    /**
     * @brief Performs a raw copy of the data in source and places it into destination.
     * The destination must be equal to or larger than source.
     * @param source - The image you want copying.
     * @param destination - the image you want to paste into.
     */
    void copyTexture2D(const TextureBufferObject &source, const TextureBufferObject &destination);
    
    /**
     * @brief Pushes a grouping for debuggers like RenderDoc. For instance, we can wrap our geometry calls to see how
     * long they all take.
     * @param message - The name of the group.
     */
    void pushDebugGroup(std::string_view message);
    void popDebugGroup();
}
