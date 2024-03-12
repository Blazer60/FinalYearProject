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
    
    std::filesystem::path makeRelativeToResourcePath(const std::filesystem::path &path);
    std::filesystem::path constructAbsolutePath(const std::string &relativePath);
    
    bool hasImageExtension(const std::string &extension);
    bool hasImageExtension(const std::filesystem::path &path);
    
    bool hasModelExtension(const std::string &extension);
    bool hasModelExtension(const std::filesystem::path &path);
    
    bool hasSceneExtension(const std::string &extension);
    bool hasSceneExtension(const std::filesystem::path &path);

    bool hasSoundExtension(const std::string &extenstion);
    bool hasSoundExtension(const std::filesystem::path& path);

    bool hasMaterialLayerExtension(const std::string &extenstion);
    bool hasMaterialLayerExtension(const std::filesystem::path& path);

    bool hasMaterialExtension(const std::string &extenstion);
    bool hasMaterialExtension(const std::filesystem::path& path);
}
