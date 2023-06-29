/**
 * @file WindowHelpers.cpp
 * @author Ryan Purse
 * @date 14/05/2022
 */


#include "WindowHelpers.h"
#include "glfw3.h"

namespace window
{
    glm::ivec2 bufferSize_impl { 1920, 1080 };
    
    glm::ivec2 viewSize()
    {
        glm::ivec2 size;
        glfwGetWindowSize(glfwGetCurrentContext(), &size.x, &size.y);
        return size;
    }
    
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
}