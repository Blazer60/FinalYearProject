/**
 * @file Cubemap.h
 * @author Ryan Purse
 * @date 26/06/2023
 */


#pragma once

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
    ~Cubemap();
    
    [[nodiscard]] uint32_t getId() const { return mId; }
    
protected:
    uint32_t mId;
};
