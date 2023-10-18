/**
 * @file WindowHelpers.h
 * @author Ryan Purse
 * @date 14/05/2022
 */


#pragma once

#include "Pch.h"

namespace window
{
    /**
     * @brief Sets the size of the window that the renderer will render to.
     * @param size - The new size of the window.
     */
    void setBufferSize(const glm::ivec2 &size);
    
    /**
     * @returns - The size that the renderer is currently rendering to.
     */
    glm::ivec2 bufferSize();
    
    /**
     * @returns - window width / widow height (useful for creating perspective matrices).
     */
    float aspectRatio();
}
