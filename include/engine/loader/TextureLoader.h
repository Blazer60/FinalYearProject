/**
 * @file TextureLoader.h
 * @author Ryan Purse
 * @date 13/03/2022
 */


#pragma once

#include <string_view>
#include "Pch.h"
#include "Texture.h"

namespace load
{
    std::shared_ptr<Texture> texture(std::string_view path);
}
