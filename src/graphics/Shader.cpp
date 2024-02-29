/**
 * @file Shader.cpp
 * @author Ryan Purse
 * @date 13/02/2022
 */


#include "Shader.h"
#include "gtc/type_ptr.hpp"
#include <Statistics.h>
#include <fstream>
#include <sstream>

#include "shader/ShaderCompilation.h"


Shader::Shader(const std::vector<std::filesystem::path>& paths, const std::vector<graphics::Definition>& definitions)
    : mId(glCreateProgram())
{
    if (!paths.empty())
    {
        mDebugName = std::prev(paths.end())->filename().string();
        setDebugName(mDebugName);
    }

    CreateShaderSource(paths, definitions);
}

Shader::~Shader()
{
    if (mId != 0)
        glDeleteProgram(mId);
}

void Shader::bind() const
{
    glUseProgram(mId);
}

void Shader::unbind()
{
    glUseProgram(0);
}

int Shader::getLocation(const std::string &name)
{
    if (mCache.count(name) > 0)
        return mCache.at(name);
    
    const int location = glGetUniformLocation(mId, name.data());
    if (location == -1)
        WARN("Uniform '%' does not exist! (%)", name, mDebugName);
    
    mCache[name] = location;
    return location;
}

void Shader::set(const std::string &uniformName, const int value)
{
    glProgramUniform1i(mId, getLocation(uniformName), value);
}

void Shader::set(const std::string& uniformName, const unsigned int value)
{
    glProgramUniform1i(mId, getLocation(uniformName), static_cast<int>(value));
}

void Shader::set(const std::string &uniformName, const float value)
{
    glProgramUniform1f(mId, getLocation(uniformName), value);
}

void Shader::set(const std::string &uniformName, const glm::mat4 &value)
{
    glProgramUniformMatrix4fv(mId, getLocation(uniformName), 1, GL_FALSE, glm::value_ptr(value));
}

void Shader::set(const std::string &uniformName, const glm::vec4 &value)
{
    glProgramUniform4fv(mId, getLocation(uniformName), 1, glm::value_ptr(value));
}

void Shader::set(const std::string &uniformName, const glm::vec3 &value)
{
    glProgramUniform3fv(mId, getLocation(uniformName), 1, glm::value_ptr(value));
}

void Shader::set(const std::string &uniformName, const glm::vec2 &value)
{
    glProgramUniform2fv(mId, getLocation(uniformName), 1, glm::value_ptr(value));
}

void Shader::set(const std::string &uniformName, const uint32_t textureId, const int bindPoint)
{
    glActiveTexture(GL_TEXTURE0 + bindPoint);
    glBindTextureUnit(bindPoint, textureId);
    set(uniformName, bindPoint);
}

void Shader::set(const std::string &uniformName, const float *values, const int count)
{
    glProgramUniform1fv(mId, getLocation(uniformName), count, values);
}

void Shader::set(const std::string &uniformName, const glm::mat4 *values, const int count)
{
    glProgramUniformMatrix4fv(mId, getLocation(uniformName), count, GL_FALSE, glm::value_ptr(values[0]));
}

void Shader::image(const std::string &uniformName, const uint32_t textureId, const GLenum textureFormat, const int bindPoint, const bool isArrayOr3D, const uint32_t permissions, const int level)
{
    if (textureFormat == GL_RGB16F || textureFormat == GL_RGB32F)
        WARN("Images do not support 3 component vectors.");

    glBindImageTexture(bindPoint, textureId, level, isArrayOr3D ? GL_TRUE : GL_FALSE, 0, permissions, textureFormat);
    set(uniformName, bindPoint);
}

void Shader::block(const std::string& blockName, const unsigned int blockBindPoint)
{
    const auto blockIndex = glGetUniformBlockIndex(mId, blockName.c_str());
    if (blockIndex == GL_INVALID_INDEX)
        WARN("Block with name % does not exists in shader %", blockName, mDebugName);

    glUniformBlockBinding(mId, blockIndex, blockBindPoint);
}

void Shader::setDebugName(const std::string_view name) const
{
    glObjectLabel(GL_PROGRAM, mId, -1, name.data());
}

void Shader::validateProgram() const
{
    glValidateProgram(mId);
    int result { 0 };
    if (glGetProgramiv(mId, GL_VALIDATE_STATUS, &result); result == GL_TRUE)
        return;

    // Program stage failed.
    int length { 0 };
    glGetProgramiv(mId, GL_INFO_LOG_LENGTH, &length);
    char *message = static_cast<char*>(alloca(length * sizeof(char)));
    glGetProgramInfoLog(mId, length, &length, message);
    glDeleteProgram(mId);

    CRASH("Failed to validate program (%): \n%\n(Make sure that samplers have layout(binding = x) in front of them.)", mDebugName, message);
}

void Shader::CreateShaderSource(const std::initializer_list<std::filesystem::path> paths) const
{
    std::vector<unsigned int> shaderIds;
    for (const std::filesystem::path &path : paths)
        shaderIds.push_back(graphics::compileShader(path, { }));

    for (const unsigned int shaderId : shaderIds)
        glAttachShader(mId, shaderId);

    glLinkProgram(mId);
    validateProgram();

    for (const unsigned int shaderId : shaderIds)
        glDeleteShader(shaderId);
}

void Shader::CreateShaderSource(
    const std::vector<std::filesystem::path>& paths, const std::vector<graphics::Definition>& macros) const
{
    std::vector<unsigned int> shaderIds;
    for (const std::filesystem::path &path : paths)
        shaderIds.push_back(compileShader(path, macros));

    for (const unsigned int shaderId : shaderIds)
        glAttachShader(mId, shaderId);

    glLinkProgram(mId);
    validateProgram();

    for (const unsigned int shaderId : shaderIds)
        glDeleteShader(shaderId);
}
