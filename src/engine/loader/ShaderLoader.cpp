/**
 * @file ShaderLoader.cpp
 * @author Ryan Purse
 * @date 01/11/2023
 */


#include "ShaderLoader.h"
#include "EngineState.h"
#include "ResourcePool.h"

[[nodiscard]] std::shared_ptr<Shader> load::shader(
    const std::filesystem::path &vertexPath,
    const std::filesystem::path &fragmentPath)
{
    return engine::resourcePool->loadShader(vertexPath, fragmentPath);
}
