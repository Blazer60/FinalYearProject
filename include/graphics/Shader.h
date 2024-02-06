/**
 * @file Shader.h
 * @author Ryan Purse
 * @date 13/02/2022
 */


#pragma once

#include "Pch.h"
#include <filesystem>

/**
 * Holds and compiles the shader program for OpenGL.
 * @author Ryan Purse
 * @date 13/02/2022
 */
class Shader
{
public:
    Shader(std::initializer_list<std::filesystem::path> paths);
    Shader(const std::filesystem::path &vertexPath, const std::filesystem::path &fragmentPath);
    virtual ~Shader();
    
    void bind() const;
    static void unbind();
    
    /**
     * @brief Sets a uniform within the shader.
     * @param uniformName - The name within the shader.
     * @param value - The value you want to pass to it.
     */
    void set(const std::string &uniformName, int value);
    
    /**
     * @brief Sets a uniform within the shader.
     * @param uniformName - The name within the shader.
     * @param value - The value you want to pass to it.
     */
    void set(const std::string &uniformName, float value);
    
    /**
     * @brief Sets a uniform within the shader.
     * @param uniformName - The name within the shader.
     * @param value - The value you want to pass to it.
     */
    void set(const std::string &uniformName, const glm::mat4 &value);
    
    /**
     * @brief Sets a uniform within the shader.
     * @param uniformName - The name within the shader.
     * @param value - The value you want to pass to it.
     */
    void set(const std::string &uniformName, const glm::vec4 &value);
    
    /**
     * @brief Sets a uniform within the shader.
     * @param uniformName - The name within the shader.
     * @param value - The value you want to pass to it.
     */
    void set(const std::string &uniformName, const glm::vec3 &value);
    
    /**
     * @brief Sets a uniform within the shader.
     * @param uniformName - The name within the shader.
     * @param value - The value you want to pass to it.
     */
    void set(const std::string &uniformName, const glm::vec2 &value);
    
    /**
     * @brief Sets a uniforms for a texture within the shader.
     * @param uniformName - The name within the shader.
     * @param textureId - The value you want to pass to it.
     * @param bindPoint - The binding point of the texture [0-8].
     */
    void set(const std::string &uniformName, const uint32_t textureId, const int bindPoint);
    
    /**
     * @brief Sets a uniform array of floats.
     * @param uniformName - The name within the shader.
     * @param value - The pointer to the first value in the list.
     * @param count - The number of elements in the array.
     */
    void set(const std::string &uniformName, const float* values, int count);
    
    void set(const std::string &uniformName, const glm::mat4 *values, int count);

    void block(const std::string &blockName, unsigned int blockBindPoint);
    
    void setDebugName(std::string_view name) const;

    void validateProgram() const;

protected:

    std::string mDebugName;
    unsigned int mId { 0 };
    std::unordered_map<std::string , int> mCache;
    
    int getLocation(const std::string &name);

    void CreateShaderSource(std::initializer_list<std::filesystem::path> paths) const;
};


