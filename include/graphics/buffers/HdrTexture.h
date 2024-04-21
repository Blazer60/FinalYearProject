/**
 * @file HdrTexture.h
 * @author Ryan Purse
 * @date 11/07/2023
 */


#pragma once

#include "Pch.h"


/**
 * @author Ryan Purse
 * @date 11/07/2023
 */
class HdrTexture
{
public:
    explicit HdrTexture(std::string_view path);
    ~HdrTexture();
    
    [[nodiscard]] uint32_t getId() const { return mId; }
    void setDebugName(std::string_view name) const;
    
protected:
    uint32_t mId;
};

