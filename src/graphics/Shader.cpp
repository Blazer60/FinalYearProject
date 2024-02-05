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


Shader::Shader(const std::initializer_list<std::filesystem::path> paths)
    : mId(glCreateProgram())
{
    if (paths.size() != 0)
    {
        mDebugName = std::prev(paths.end())->filename().string();
        setDebugName(mDebugName);
    }

    CreateShaderSource(paths);
}

Shader::Shader(const std::filesystem::path &vertexPath, const std::filesystem::path &fragmentPath)
    : mDebugName(vertexPath.filename().string() + " | " + fragmentPath.filename().string()), mId(glCreateProgram())
{
    CreateShaderSource({ vertexPath, fragmentPath });
    setDebugName(mDebugName);
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

    ERROR("Failed to validate program (%): \n%\n(Make sure that samplers have layout(binding = x) in front of them.)", mDebugName, message);
}

void Shader::CreateShaderSource(const std::initializer_list<std::filesystem::path> paths) const
{
    std::vector<unsigned int> shaderIds;
    for (const std::filesystem::path &path : paths)
        shaderIds.push_back(graphics::compileShader(path));

    for (const unsigned int shaderId : shaderIds)
        glAttachShader(mId, shaderId);

    glLinkProgram(mId);
    validateProgram();

    for (const unsigned int shaderId : shaderIds)
        glDeleteShader(shaderId);
}
