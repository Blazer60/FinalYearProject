/**
 * @file WindowHelpers.h
 * @author Ryan Purse
 * @date 14/05/2022
 */


#pragma once

#include "Pch.h"

namespace window
{
    extern glm::ivec2 bufferSize_impl;
    
    glm::ivec2 viewSize();
    
    void setBufferSize(const glm::ivec2 &size);
    glm::ivec2 bufferSize();
}
