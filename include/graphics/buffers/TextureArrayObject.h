/**
 * @file TextureArrayObject.h
 * @author Ryan Purse
 * @date 29/06/2023
 */


#pragma once

#include "Pch.h"
#include "RendererHelpers.h"


/**
 * Similar to a TextureBufferObject but holds an array of them instead. All elements must be of the same size.
 * @author Ryan Purse
 * @date 29/06/2023
 */
class TextureArrayObject
{
    friend class FramebufferObject;
public:
    /**
     * @param layers - The number of textures in the array.
     */
    TextureArrayObject(const glm::ivec2 &size, int32_t layers, GLenum format, renderer::Filter filterMode, renderer::Wrap wrapMode);
    ~TextureArrayObject();
    
    void setBorderColour(const glm::vec4 &colour) const;
    
    [[nodiscard]] uint32_t getId() const;
    [[nodiscard]] glm::ivec2 getSize() const;
    [[nodiscard]] int32_t getLayerCount() const;
    
protected:
    uint32_t mId { 0 };
    GLenum mFormat { GL_RGB16 };
    glm::ivec2 mSize { 1024 };
    int32_t mLayers { 1 };
};
