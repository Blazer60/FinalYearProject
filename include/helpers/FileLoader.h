/**
 * @file FileLoader.h
 * @author Ryan Purse
 * @date 08/10/2023
 */


#pragma once

#include "Pch.h"
#include <filesystem>

namespace file
{
    bool findResourceFolder();
    
    std::filesystem::path resourcePath();
    std::filesystem::path modelPath();
    std::filesystem::path shaderPath();
    std::filesystem::path texturePath();
}
