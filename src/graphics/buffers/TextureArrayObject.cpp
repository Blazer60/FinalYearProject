/**
 * @file TextureArrayObject.cpp
 * @author Ryan Purse
 * @date 29/06/2023
 */


#include "TextureArrayObject.h"
#include "gtc/type_ptr.hpp"

void TextureArrayObject::init()
{
    const auto filter = toGLint(mFilter);
    const auto wrap = toGLint(mWrap);

    glCreateTextures(GL_TEXTURE_2D_ARRAY, 1, &mId);
    glTextureParameteri(mId, GL_TEXTURE_MIN_FILTER, filter);
    glTextureParameteri(mId, GL_TEXTURE_MAG_FILTER, filter);
    glTextureParameteri(mId, GL_TEXTURE_WRAP_S, wrap);
    glTextureParameteri(mId, GL_TEXTURE_WRAP_T, wrap);

    constexpr int mipMapLevels = 1;
    glTextureStorage3D(mId, mipMapLevels, mFormat, mSize.x, mSize.y, mLayers);
}

TextureArrayObject::TextureArrayObject(
    const glm::ivec2 &size, const int32_t layers, const GLenum format, const graphics::filter filterMode, const graphics::wrap wrapMode)
    : mFormat(format), mSize(size), mLayers(layers), mFilter(filterMode), mWrap(wrapMode)
{
    init();
}

TextureArrayObject::TextureArrayObject(const graphics::textureFormat format, const int32_t layers)
    : mFormat(toGLenum(format)), mLayers(layers)
{
}

void TextureArrayObject::deInit()
{
    if (mId == 0)
        glDeleteTextures(1, &mId);
    mId = 0;
}

TextureArrayObject::~TextureArrayObject()
{
    deInit();
}

uint32_t TextureArrayObject::getId() const
{
    return mId;
}

void TextureArrayObject::resize(const glm::ivec2& newSize)
{
    if (mSize == newSize)
        return;

    deInit();
    mSize = newSize;
    init();
}

void TextureArrayObject::setBorderColour(const glm::vec4 &colour) const
{
    glTextureParameterfv(mId, GL_TEXTURE_BORDER_COLOR, glm::value_ptr(colour));
}

void TextureArrayObject::clear(const glm::uvec4 &clearColour) const
{
    glClearTexImage(mId, 0, GL_RGBA_INTEGER, GL_UNSIGNED_INT, glm::value_ptr(clearColour));
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
