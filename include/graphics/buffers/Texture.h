/**
 * @file Texture.h
 * @author Ryan Purse
 * @date 22/06/2023
 */


#pragma once

#include <filesystem>
#include "Pch.h"



/**
 * @author Ryan Purse
 * @date 22/06/2023
 */
class Texture
{
public:
    explicit Texture(const std::filesystem::path &path);
    ~Texture();
    
    [[nodiscard]] uint32_t id() const { return mId; }
    
protected:
    uint32_t mId { 0 };
};
