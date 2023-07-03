/**
 * @file TextureArrayObject.cpp
 * @author Ryan Purse
 * @date 29/06/2023
 */


#include "TextureArrayObject.h"
#include "gtc/type_ptr.hpp"

TextureArrayObject::TextureArrayObject(
    const glm::ivec2 &size, int32_t layers, GLenum format, renderer::Filter filterMode, renderer::Wrap wrapMode)
    : mSize(size), mLayers(layers), mFormat(format)
{
    const auto filter = toGLint(filterMode);
    const auto wrap = toGLint(wrapMode);
    
    glCreateTextures(GL_TEXTURE_2D_ARRAY, 1, &mId);
    glTextureParameteri(mId, GL_TEXTURE_MIN_FILTER, filter);
    glTextureParameteri(mId, GL_TEXTURE_MAG_FILTER, filter);
    glTextureParameteri(mId, GL_TEXTURE_WRAP_S, wrap);
    glTextureParameteri(mId, GL_TEXTURE_WRAP_T, wrap);
    
    const int mipMapLevels = 1;
    glTextureStorage3D(mId, mipMapLevels, format, mSize.x, mSize.y, mLayers);
}

TextureArrayObject::~TextureArrayObject()
{
    glDeleteTextures(1, &mId);
}

uint32_t TextureArrayObject::getId() const
{
    return mId;
}

void TextureArrayObject::setBorderColour(const glm::vec4 &colour) const
{
    glTextureParameterfv(mId, GL_TEXTURE_BORDER_COLOR, glm::value_ptr(colour));
}

glm::ivec2 TextureArrayObject::getSize() const
{
    return mSize;
}

int32_t TextureArrayObject::getLayerCount() const
{
    return mLayers;
}

GLenum TextureArrayObject::getFormat() const
{
    return mFormat;
}

namespace renderer
{
    std::unique_ptr<TextureBufferObject> cloneTextureLayer(const TextureArrayObject &from, int layer)
    {
        auto result = std::make_unique<TextureBufferObject>(from.getSize(), from.getFormat(), Filter::Linear, Wrap::Repeat);
        const int x = 0;
        const int y = 0;
        const int z = 0;
        const int mipLevel = 0;
        glCopyImageSubData(from.getId(), GL_TEXTURE_2D_ARRAY, mipLevel, x, y, layer, result->getId(), GL_TEXTURE_2D, mipLevel, x, y, z, result->getSize().x, result->getSize().y, 1);
        return result;
    }
}
