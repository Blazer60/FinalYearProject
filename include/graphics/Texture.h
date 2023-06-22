/**
 * @file Texture.h
 * @author Ryan Purse
 * @date 22/06/2023
 */


#pragma once

#include "Pch.h"



/**
 * @author Ryan Purse
 * @date 22/06/2023
 */
class Texture
{
public:
    explicit Texture(std::string_view path);
    ~Texture();
    
    [[nodiscard]] int id() const { return static_cast<int>(mId); }
    
protected:
    uint32_t mId { 0 };
};
