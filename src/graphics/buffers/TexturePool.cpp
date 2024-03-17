/**
 * @file TexturePool.cpp
 * @author Ryan Purse
 * @date 16/03/2024
 */


#include "TexturePool.h"

#include "GraphicsFunctions.h"

namespace graphics
{
    TexturePool::TexturePool(const std::string& debugName, const textureFormat format, const filter filter)
        : mDebugName(debugName), mFormat(format), mFilter(filter)
    {
    }

    TexturePool::~TexturePool()
    {
        if (mId != 0)
            glDeleteTextures(1, &mId);
        mId = 0;
    }

    int32_t TexturePool::addTexture(const Texture& texture)
    {
        if (texture.id() == 0)
            return -1;

        const glm::ivec2 textureSize = texture.size();
        const int32_t dstIndex = findFirstAvailableSlot();

        const glm::ivec2 newSize = glm::max(getMaxTextureSize(), textureSize);
        const int32_t newCount = glm::max(dstIndex + 1, findLastUsedSlot());
        MESSAGE("Adding Texture to index %. Size: % -> %. Count: % -> %", dstIndex, mSize, newSize, mLayerCount, newCount);
        reinitialise(newSize, newCount);

        constexpr int x = 0;
        constexpr int y = 0;
        constexpr int z = 0;
        constexpr int mipLevel = 0;

        glCopyImageSubData(
            texture.id(), GL_TEXTURE_2D, mipLevel, x, y, z,
            mId, GL_TEXTURE_2D_ARRAY, mipLevel, x, y, dstIndex,
            textureSize.x, textureSize.y, 1
        );

        mData[dstIndex].width  = textureSize.x;
        mData[dstIndex].height = textureSize.y;

        return dstIndex;
    }

    void TexturePool::removeTexture(const int32_t index)
    {
        if (index == -1)
            return;

        MESSAGE("Deleting Texture at index %. Count: %", index, mLayerCount);
        mData[index] = { };
    }

    void TexturePool::reinitialise(const glm::ivec2 newSize, const int32_t newCount)
    {
        if (mSize == newSize && mLayerCount == newCount)
            return;

        uint32_t newId = 0;
        glCreateTextures(GL_TEXTURE_2D_ARRAY, 1, &newId);

        glTextureParameteri(newId, GL_TEXTURE_MIN_FILTER, toGLint(mFilter));
        glTextureParameteri(newId, GL_TEXTURE_MAG_FILTER, toMagGLint(mFilter));
        glTextureParameteri(newId, GL_TEXTURE_WRAP_S, toGLint(mWrap));
        glTextureParameteri(newId, GL_TEXTURE_WRAP_T, toGLint(mWrap));

        constexpr int mipMapLevels = 1;
        glTextureStorage3D(newId, mipMapLevels, toGLenum(mFormat), newSize.x, newSize.y, newCount);

        if (!mDebugName.empty())
            glObjectLabel(GL_TEXTURE, newId, static_cast<GLsizei>(mDebugName.size()), mDebugName.data());

        if (mId != 0)
        {
            const glm::ivec2 minimumSize = glm::min(mSize, newSize);
            const int32_t minimumCount = glm::min(mLayerCount, newCount);

            constexpr int x = 0;
            constexpr int y = 0;
            constexpr int z = 0;
            constexpr int mipLevel = 0;

            glCopyImageSubData(
                mId, GL_TEXTURE_2D_ARRAY, mipLevel, x, y, z,
                newId, GL_TEXTURE_2D_ARRAY, mipLevel, x, y, z,
                minimumSize.x, minimumSize.y, minimumCount
            );
        }

        glDeleteTextures(1, &mId);
        mId = newId;
        mSize = newSize;
        mLayerCount = newCount;
        mData.resize(mLayerCount);
    }

    int32_t TexturePool::findFirstAvailableSlot() const
    {
        for (int i = 0; i < mData.size(); ++i)
        {
            if (mData[i].width == 0 && mData[i].height == 0)
                return i;
        }
        return static_cast<int32_t>(mData.size());
    }

    int32_t TexturePool::findLastUsedSlot() const
    {
        for (int i = mData.size() - 1; i >= 0; --i)
        {
            if (mData[i].width != 0 && mData[i].height != 0)
                return i;
        }
        return 0;
    }

    glm::ivec2 TexturePool::getMaxTextureSize() const
    {
        glm::ivec2 result = glm::ivec2(0);
        for (auto &[width, height] : mData)
            result = glm::max(result, glm::ivec2(width, height));

        return result;
    }
} // graphics
