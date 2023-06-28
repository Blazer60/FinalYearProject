/**
 * @file UiHelpers.h
 * @author Ryan Purse
 * @date 27/06/2023
 */


#pragma once

#include "Pch.h"
#include "TextureBufferObject.h"

namespace ui
{
    void showTextureBuffer( const std::string &name, const TextureBufferObject &texture, bool *show, bool isMainBuffer);
}
