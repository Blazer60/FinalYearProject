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
    explicit Texture(Texture &other) = delete;
    ~Texture();
    
    [[nodiscard]] uint32_t id() const { return mId; }
    [[nodiscard]] glm::ivec2 size() const { return mSize; }
    [[nodiscard]] std::filesystem::path path() const { return mPath; }
    void setData(const glm::ivec2 &size, const unsigned char *bytes);

    std::filesystem::path mPath;
    uint32_t mId { 0 };
    glm::ivec2 mSize { 0 };
};
