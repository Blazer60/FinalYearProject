/**
 * @file Texture3DObject.h
 * @author Ryan Purse
 * @date 29/02/2024
 */


#pragma once

#include "GraphicsDefinitions.h"
#include "Pch.h"

namespace graphics
{
    /**
     * @author Ryan Purse
     * @date 29/02/2024
     */
    class Texture3DObject
    {
    public:
        explicit Texture3DObject(Texture3DObject &rhs) = delete;
        ~Texture3DObject();

        Texture3DObject(const glm::ivec3 &size, textureFormat format, filter filter, wrap wrapMode, uint32_t mipLevel=1);
        Texture3DObject(const glm::ivec3 &size, textureFormat format);

        [[nodiscard]] unsigned int getId() const;
        [[nodiscard]] glm::ivec3 getSize() const;
        [[nodiscard]] GLenum getFormat() const;

        void clear(const glm::vec4 &clearColour=glm::vec4(0.f, 0.f, 0.f, 1.f)) const;
        void setDebugName(const std::string &name) const;

    protected:
        void init(const glm::ivec3 &size, textureFormat format, filter filter, wrap wrapMode, uint32_t mipLevels);

        unsigned int mId = 0;
        textureFormat mFormat = textureFormat::Rgba16f;
        glm::ivec3 mSize = glm::ivec3(256);
        uint32_t mMipLevels = 1;
    };
} // graphics
