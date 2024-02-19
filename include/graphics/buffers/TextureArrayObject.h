/**
 * @file TextureArrayObject.h
 * @author Ryan Purse
 * @date 29/06/2023
 */


#pragma once

#include "Pch.h"
#include "GraphicsDefinitions.h"


/**
 * Similar to a TextureBufferObject but holds an array of them instead. All elements must be of the same size.
 * @author Ryan Purse
 * @date 29/06/2023
 */
class TextureArrayObject
{
public:
    /**
     * @param layers - The number of textures in the array.
     */
    TextureArrayObject(const glm::ivec2 &size, int32_t layers, GLenum format, graphics::filter filterMode, graphics::wrap wrapMode);
    ~TextureArrayObject();
    
    void setBorderColour(const glm::vec4 &colour) const;
    void clear(const glm::uvec4 &clearColour=glm::uvec4(0)) const;
    
    [[nodiscard]] uint32_t getId() const;
    [[nodiscard]] glm::ivec2 getSize() const;
    [[nodiscard]] int32_t getLayerCount() const;
    [[nodiscard]] GLenum getFormat() const;
    
protected:
    uint32_t mId { 0 };
    GLenum mFormat { GL_RGB16 };
    glm::ivec2 mSize { 1024 };
    int32_t mLayers { 1 };
};
