/**
 * @file ShaderLoader.h
 * @author Ryan Purse
 * @date 01/11/2023
 */


#pragma once

#include "Pch.h"
#include "Shader.h"

namespace load
{
    std::shared_ptr<Shader> shader(const std::filesystem::path &vertexPath, const std::filesystem::path &fragmentPath);
}