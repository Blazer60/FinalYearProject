/**
 * @file TexturePool.h
 * @author Ryan Purse
 * @date 16/03/2024
 */


#pragma once

#include <queue>
#include <stack>

#include "GraphicsDefinitions.h"
#include "MaterialData.h"
#include "Texture.h"

namespace graphics
{
    /**
     * @author Ryan Purse
     * @date 16/03/2024
     */
    class TexturePool 
    {
    public:
        TexturePool(const std::string &debugName, textureFormat format, filter filter=filter::LinearMipmapLinear);
        ~TexturePool();
        uint32_t id() const { return mId; }
        std::vector<TextureData> data() const { return mData; }

        int32_t addTexture(const Texture &texture);
        void removeTexture(int32_t index);

    protected:
        void reinitialise(glm::ivec2 newSize, int32_t newCount);
        int32_t findFirstAvailableSlot() const;
        int32_t findLastUsedSlot() const;
        glm::ivec2 getMaxTextureSize() const;

        std::string     mDebugName;
        uint32_t        mId         = 0;
        textureFormat   mFormat     = textureFormat::Rgba8;
        filter          mFilter     = filter::LinearMipmapLinear;
        wrap            mWrap       = wrap::ClampToEdge;
        glm::ivec2      mSize       = glm::ivec2(0);
        int32_t         mLayerCount = 0;
        std::vector<TextureData> mData;
    };

} // graphics
