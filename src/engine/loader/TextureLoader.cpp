/**
 * @file TextureLoader.cpp
 * @author Ryan Purse
 * @date 13/03/2022
 */


#include <filesystem>
#include "TextureLoader.h"
#include "stb_image.h"

#include <Statistics.h>

namespace load
{
    std::shared_ptr<Texture> texture(std::string_view path)
    {
        return std::make_shared<Texture>(path);
    }
    
    GLFWimage windowIcon(std::string_view path)
    {
        stbi_set_flip_vertically_on_load(false);
        std::filesystem::path systemPath(path);
        if (!std::filesystem::exists(systemPath))
        {
            WARN("File % does not exist. The Window Icon's data will be null.", systemPath);
            return GLFWimage { };
        }
        
        int width;
        int height;
        int colourChannels;
        // Forcing 4 components so that is always aligns with opengl's internal format.
        unsigned char *bytes = stbi_load(systemPath.string().c_str(), &width, &height, &colourChannels, 4);
        
        GLFWimage icon;
        icon.pixels = bytes;
        icon.width = width;
        icon.height = height;
        
        return icon;
    }
}


