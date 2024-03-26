/**
 * @file Disk.h
 * @author Ryan Purse
 * @date 26/03/2024
 */


#pragma once

#include "Pch.h"
#include "Texture.h"

// Loading that takes a while.
namespace engine::disk
{
    struct StbiTextureData
    {
        int width = 0;
        int height = 0;
        int colourChannels = 4;
        unsigned char *bytes = nullptr;
    };

    StbiTextureData image(const std::filesystem::path &path);
    void release(StbiTextureData &data);
}
