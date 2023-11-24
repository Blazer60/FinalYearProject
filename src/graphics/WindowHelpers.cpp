/**
 * @file WindowHelpers.cpp
 * @author Ryan Purse
 * @date 14/05/2022
 */


#include "WindowHelpers.h"

namespace window
{
    static glm::ivec2 bufferSize_impl { 1920, 1080 };
    
    void setBufferSize(const glm::ivec2 &size)
    {
        bufferSize_impl = size;
    }
    
    glm::ivec2 bufferSize()
    {
        return bufferSize_impl;
    }
    
    float aspectRatio()
    {
        return static_cast<float>(bufferSize_impl.x) / static_cast<float>(bufferSize_impl.y);
    }

    int maximumMipLevel()
    {
        return glm::floor(glm::log2(static_cast<float>(glm::max(bufferSize_impl.x, bufferSize_impl.y))));
    }
}
