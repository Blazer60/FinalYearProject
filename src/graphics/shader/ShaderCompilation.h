/**
 * @file ShaderCompilation.h
 * @author Ryan Purse
 * @date 05/02/2024
 */


#pragma once

#include <filesystem>

#include "Pch.h"

namespace graphics
{
    struct ShaderData
    {
        std::filesystem::path path;
        std::vector<std::filesystem::path> includePaths;
        std::string source;
    };

    unsigned int getGlslType(const std::filesystem::path &path);
    unsigned int compileShader(const std::filesystem::path &path);

    unsigned int compileShaderSource(unsigned int shaderType, const std::list<ShaderData> &data);
    ShaderData passShader(const std::filesystem::path& path);
}
