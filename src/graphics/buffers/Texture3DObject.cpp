/**
 * @file Texture3DObject.cpp
 * @author Ryan Purse
 * @date 29/02/2024
 */


#include "Texture3DObject.h"

#include "WindowHelpers.h"

namespace graphics
{
    Texture3DObject::~Texture3DObject()
    {
        glDeleteTextures(1, &mId);
        mId = 0;
    }

    Texture3DObject::Texture3DObject(
        const glm::ivec3& size, const textureFormat format, const filter filter, const  wrap wrapMode, const uint32_t mipLevel)
    {
        init(size, format, filter, wrapMode, mipLevel);
    }

    Texture3DObject::Texture3DObject(const glm::ivec3& size, const textureFormat format)
    {
        init(size, format, filter::Linear, wrap::ClampToEdge, 1);
    }

    unsigned int Texture3DObject::getId() const
    {
        return mId;
    }

    glm::ivec3 Texture3DObject::getSize() const
    {
        return mSize;
    }

    GLenum Texture3DObject::getFormat() const
    {
        return toGLenum(mFormat);
    }

    void Texture3DObject::clear(const glm::vec4& clearColour) const
    {
        glClearTexImage(mId, 0, GL_RGBA, GL_FLOAT, glm::value_ptr(clearColour));
    }

    void Texture3DObject::setDebugName(const std::string& name) const
    {
        glObjectLabel(GL_TEXTURE, mId, -1, name.data());
    }

    void Texture3DObject::init(
        const glm::ivec3& size, const textureFormat format, const filter filter,
        const  wrap wrapMode, const uint32_t mipLevels)
    {
        mSize = size;
        mFormat = format;
        mMipLevels = mipLevels;

        if (const int maximumMipLevel = window::maximumMipLevel(); mMipLevels > maximumMipLevel)
        {
            WARN("A Texture Buffer Object requested % mip levels, but could make % mip levels. "
                 "This may cause undefined behaviour.", mMipLevels, maximumMipLevel);
            mMipLevels = maximumMipLevel;
        }

        glCreateTextures(GL_TEXTURE_3D, 1, &mId);
        glTextureParameteri(mId, GL_TEXTURE_MIN_FILTER, toGLint(filter));
        glTextureParameteri(mId, GL_TEXTURE_MAG_FILTER, toMagGLint(filter));

        glTextureParameteri(mId, GL_TEXTURE_WRAP_S, toGLint(wrapMode));
        glTextureParameteri(mId, GL_TEXTURE_WRAP_T, toGLint(wrapMode));
        glTextureParameteri(mId, GL_TEXTURE_WRAP_R, toGLint(wrapMode));

        glTextureStorage3D(
            mId, static_cast<int>(mMipLevels), toGLenum(mFormat),
            mSize.x, mSize.y, mSize.z);
    }
} // graphics
