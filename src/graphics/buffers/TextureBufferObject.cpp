/**
 * @file TextureBufferObject.cpp
 * @author Ryan Purse
 * @date 14/03/2022
 */


#include "TextureBufferObject.h"

TextureBufferObject::TextureBufferObject(const glm::ivec2 &size)
    : mSize(size)
{
    init(GL_NEAREST, GL_LINEAR);
}

TextureBufferObject::TextureBufferObject(
    const glm::ivec2 &size, GLenum format, GLenum minFilter, GLenum magFilter, const uint32_t mipmapLevels,
    std::string debugName)
    :
    mSize(size), mFormat(format), mDebugName(std::move(debugName)), mMipMapLevels(mipmapLevels)
{
    init(minFilter, magFilter);
}

TextureBufferObject::~TextureBufferObject()
{
    deInit();
}

void TextureBufferObject::init(const GLenum minFilter, const GLenum magFilter)
{
    glCreateTextures(GL_TEXTURE_2D, 1, &mId);
    glTextureParameteri(mId, GL_TEXTURE_MIN_FILTER, minFilter);
    glTextureParameteri(mId, GL_TEXTURE_MAG_FILTER, magFilter);
    glTextureParameteri(mId, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTextureParameteri(mId, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
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

