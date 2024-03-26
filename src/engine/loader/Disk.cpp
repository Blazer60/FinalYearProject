/**
 * @file Disk.cpp
 * @author Ryan Purse
 * @date 26/03/2024
 */


#include "Disk.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

namespace engine::disk
{
    StbiTextureData image(const std::filesystem::path& path)
    {
        stbi_set_flip_vertically_on_load(1);

        const std::string pathString = path.string();

        StbiTextureData image;
        image.bytes = stbi_load(pathString.c_str(), &image.width, &image.height, &image.colourChannels, 4);
        return image;
    }

    void release(StbiTextureData& data)
    {
        stbi_image_free(data.bytes);
        data.bytes = nullptr;
    }
}
