/**
 * @file TextureLoader.cpp
 * @author Ryan Purse
 * @date 13/03/2022
 */


#include "TextureLoader.h"

namespace load
{
    std::shared_ptr<Texture> texture(std::string_view path)
    {
        return std::make_shared<Texture>(path);
    }
}


