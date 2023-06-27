/**
 * @file TextureBufferObject.cpp
 * @author Ryan Purse
 * @date 14/03/2022
 */


#include "TextureBufferObject.h"
#include "gtc/type_ptr.hpp"

TextureBufferObject::TextureBufferObject(const glm::ivec2 &size)
    : mSize(size)
{
    init(GL_NEAREST, GL_LINEAR, GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE);
}

TextureBufferObject::TextureBufferObject(
    const glm::ivec2 &size, GLenum format, GLint minFilter, GLint magFilter, const uint32_t mipmapLevels,
    std::string debugName)
    :
    mSize(size), mFormat(format), mDebugName(std::move(debugName)), mMipMapLevels(mipmapLevels)
{
    init(minFilter, magFilter, GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE);
}

TextureBufferObject::TextureBufferObject(
    const glm::ivec2 &size, const GLenum format, renderer::Filter filterMode,
    renderer::Wrap wrapMode, uint32_t mipmapLevel, std::string debugName)
    :
    mSize(size), mFormat(format), mDebugName(std::move(debugName)), mMipMapLevels(mipmapLevel)
{
    const auto filter = toGLint(filterMode);
    const auto wrap = toGLint(wrapMode);
    init(filter, filter, wrap, wrap);
}

TextureBufferObject::~TextureBufferObject()
{
    deInit();
}

void TextureBufferObject::init(const GLint minFilter, const GLint magFilter, const GLint wrapS, const GLint wrapT)
{
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


