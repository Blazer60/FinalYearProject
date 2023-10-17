/**
 * @file TextureLoader.h
 * @author Ryan Purse
 * @date 13/03/2022
 */


#pragma once

#include <string_view>
#include "Pch.h"
#include "Texture.h"
#include "glfw3.h"

namespace load
{
    /**
     * @brief Creates a textures that can be used anywhere.
     * @param path - The path to the texture that you want to load.
     * @returns A texture object. ->id() is 0 if it failed to load the texture
     * (check the runtime logs for more info).
     */
    std::shared_ptr<Texture> texture(const std::filesystem::path &path);
    
    GLFWimage windowIcon(std::string_view path);
}
