/**
 * @file Cubemap.h
 * @author Ryan Purse
 * @date 26/06/2023
 */


#pragma once

#include "GraphicsDefinitions.h"
#include "Pch.h"


/**
 * @author Ryan Purse
 * @date 26/06/2023
 */
class Cubemap
{
public:
    /**
     * @brief Creates a cubemap texture. Cubemap textures use the left-hand rule.
     * @param paths Cubemap order [+x, -x, +y, -y, +z, -z]
     * @see <a href="https://www.khronos.org/opengl/wiki/Cubemap_Texture">Cubemap Textures</a>
     */
    explicit Cubemap(const std::vector<std::string> &paths);

    void init();

    int getMipLevels() const { return mMipLevels; }

    /**
     * @brief Creates an empty cubemap that can be rendered to.
     * @param size - The size of each face in the cubemap.
     * @param format - The format of the texture.
     */
    Cubemap(const glm::ivec2 &size, GLenum format, int mipLevels=1);

    explicit Cubemap(graphics::textureFormat format);
    Cubemap(graphics::textureFormat format, int mipLevels);

    ~Cubemap();
    
    [[nodiscard]] uint32_t   getId()     const { return mId; }
    void resize(glm::ivec2 newSize);
    [[nodiscard]] glm::ivec2 getSize()   const { return mSize; }
    [[nodiscard]] GLenum     getFormat() const { return mFormat; }
    
    void setDebugName(std::string_view name) const;
    
protected:
    void deInit();

    uint32_t    mId = 0;
    glm::ivec2  mSize = glm::ivec2(0);
    GLenum      mFormat = toGLenum(graphics::textureFormat::Rgba16f);
    int         mMipLevels = 1;
};
