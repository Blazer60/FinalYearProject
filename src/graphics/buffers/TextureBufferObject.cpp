/**
 * @file TextureBufferObject.cpp
 * @author Ryan Purse
 * @date 14/03/2022
 */


#include "TextureBufferObject.h"

#include "WindowHelpers.h"
#include "gtc/type_ptr.hpp"

TextureBufferObject::TextureBufferObject(const glm::ivec2 &size)
    : mSize(size)
{
    init(GL_NEAREST, GL_LINEAR, GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE);
}

TextureBufferObject::TextureBufferObject(
    const glm::ivec2 &size, GLenum format, GLint minFilter, GLint magFilter, uint32_t mipmapLevels)
    :
    mSize(size), mFormat(format), mMipMapLevels(mipmapLevels)
{
    init(minFilter, magFilter, GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE);
}

TextureBufferObject::TextureBufferObject(
    const glm::ivec2 &size, GLenum format, graphics::filter filterMode, graphics::wrap wrapMode,
    uint32_t mipmapLevel)
    :
    mSize(size), mFormat(format), mMipMapLevels(mipmapLevel)
{
    const auto wrap = toGLint(wrapMode);
    init(graphics::toGLint(filterMode), graphics::toMagGLint(filterMode), wrap, wrap);
}

TextureBufferObject::~TextureBufferObject()
{
    deInit();
}

void TextureBufferObject::init(const GLint minFilter, const GLint magFilter, const GLint wrapS, const GLint wrapT)
{
    if (const int maximumMipLevel = window::maximumMipLevel(); mMipMapLevels > maximumMipLevel)
    {
        WARN("A Texture Buffer Object requested % mip levels, but could make % mip levels. "
             "This may cause undefined behaviour.", mMipMapLevels, maximumMipLevel);
        mMipMapLevels = maximumMipLevel;
    }

    glCreateTextures(GL_TEXTURE_2D, 1, &mId);
    glTextureParameteri(mId, GL_TEXTURE_MIN_FILTER, minFilter);
    glTextureParameteri(mId, GL_TEXTURE_MAG_FILTER, magFilter);
    glTextureParameteri(mId, GL_TEXTURE_WRAP_S, wrapS);
    glTextureParameteri(mId, GL_TEXTURE_WRAP_T, wrapT);
    glTextureStorage2D(mId, static_cast<int>(mMipMapLevels), mFormat, mSize.x, mSize.y);
}

void TextureBufferObject::deInit()
{
    glDeleteTextures(1, &mId);
}

unsigned int TextureBufferObject::getId() const
{
    return mId;
}

const glm::ivec2 &TextureBufferObject::getSize() const
{
    return mSize;
}

void TextureBufferObject::setBorderColour(const glm::vec4 &colour) const
{
    glTextureParameterfv(mId, GL_TEXTURE_BORDER_COLOR, glm::value_ptr(colour));
}

GLenum TextureBufferObject::getFormat() const
{
    return mFormat;
}

void TextureBufferObject::generateMipMaps() const
{
    if (mMipMapLevels > 0)
    {
        glGenerateTextureMipmap(mId);
    }
}

uint32_t TextureBufferObject::getMipLevels() const
{
    return mMipMapLevels;
}
