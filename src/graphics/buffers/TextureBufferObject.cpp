/**
 * @file TextureBufferObject.cpp
 * @author Ryan Purse
 * @date 14/03/2022
 */


#include "TextureBufferObject.h"

#include "WindowHelpers.h"
#include "gtc/type_ptr.hpp"

TextureBufferObject::TextureBufferObject()
{
    // Not a valid texture until init() is called.
}

TextureBufferObject::TextureBufferObject(const glm::ivec2 &size)
    : mSize(size)
{
    init(GL_NEAREST, GL_LINEAR, GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE);
}

TextureBufferObject::TextureBufferObject(TextureBufferObject&& other) noexcept
    :
    mDebugName(other.mDebugName), mId(other.mId), mFormat(other.mFormat),
    mSize(other.mSize), mMipMapLevels(other.mMipMapLevels), mFilter(other.mFilter),
    mWrap(other.mWrap)
{
    other.mId = 0;
}

TextureBufferObject& TextureBufferObject::operator=(TextureBufferObject&& other) noexcept
{
    mDebugName = other.mDebugName;
    mId = other.mId;
    mFormat = other.mFormat;
    mSize = other.mSize;
    mMipMapLevels = other.mMipMapLevels;
    mFilter = other.mFilter;
    mWrap = other.mWrap;
    other.mId = 0;

    return *this;
}

TextureBufferObject::TextureBufferObject(
    const glm::ivec2 &size, GLenum format, GLint minFilter, GLint magFilter, uint32_t mipmapLevels)
    :
    mSize(size), mFormat(format), mMipMapLevels(mipmapLevels)
{
    init(minFilter, magFilter, GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE);
}

TextureBufferObject::TextureBufferObject(
    const glm::ivec2 &size, const GLenum format, const graphics::filter filterMode,
    const graphics::wrap wrapMode, const uint32_t mipmapLevel)
    :
    mFormat(format), mSize(size), mMipMapLevels(mipmapLevel),
    mFilter(filterMode), mWrap(wrapMode)
{
    init();
}

TextureBufferObject::TextureBufferObject(const graphics::textureFormat format)
    : mFormat(toGLenum(format))
{

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

    if (!mDebugName.empty())
        glObjectLabel(GL_TEXTURE, mId, static_cast<GLsizei>(mDebugName.size()), mDebugName.data());
}

void TextureBufferObject::init()
{
    if (const int maximumMipLevel = window::maximumMipLevel(); mMipMapLevels > maximumMipLevel)
    {
        WARN("A Texture Buffer Object requested % mip levels, but could make % mip levels. "
             "This may cause undefined behaviour.", mMipMapLevels, maximumMipLevel);
        mMipMapLevels = maximumMipLevel;
    }

    glCreateTextures(GL_TEXTURE_2D, 1, &mId);
    glTextureParameteri(mId, GL_TEXTURE_MIN_FILTER, toGLint(mFilter));
    glTextureParameteri(mId, GL_TEXTURE_MAG_FILTER, toMagGLint(mFilter));
    glTextureParameteri(mId, GL_TEXTURE_WRAP_S, toGLint(mWrap));
    glTextureParameteri(mId, GL_TEXTURE_WRAP_T, toGLint(mWrap));
    glTextureStorage2D(mId, static_cast<int>(mMipMapLevels), mFormat, mSize.x, mSize.y);

    if (!mDebugName.empty())
        glObjectLabel(GL_TEXTURE, mId, static_cast<GLsizei>(mDebugName.size()), mDebugName.data());
}

void TextureBufferObject::deInit()
{
    if (mId != 0)
        glDeleteTextures(1, &mId);
    mId = 0;
}

unsigned int TextureBufferObject::getId() const
{
    return mId;
}

void TextureBufferObject::resize(const glm::ivec2& newSize)
{
    if (mSize == newSize)
        return;

    deInit();
    mSize = newSize;
    init();
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

void TextureBufferObject::clear(const glm::vec4 &clearColour) const
{
    if (mFormat == GL_DEPTH_COMPONENT32F)
    {
        constexpr float one = 1.f;
        glClearTexImage(mId, 0, GL_DEPTH_COMPONENT, GL_FLOAT, &one);
    }
    else
        glClearTexImage(mId, 0, GL_RGBA, GL_FLOAT, glm::value_ptr(clearColour));
}

void TextureBufferObject::setDebugName(const std::string& debugName)
{
    mDebugName = debugName;
    if (mId != 0)
        glObjectLabel(GL_TEXTURE, mId, -1, debugName.data());
}

void TextureBufferObject::upload(const float* data, const graphics::pixelFormat format) const
{
    constexpr int level = 0;
    constexpr int offset = 0;
    glTextureSubImage2D(mId, level, offset, offset, mSize.x, mSize.y, toGLenum(format), GL_FLOAT, static_cast<const void*>(data));
}
