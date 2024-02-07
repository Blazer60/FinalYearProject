/**
 * @file ShaderCompilation.h
 * @author Ryan Purse
 * @date 05/02/2024
 */


#pragma once

#include <filesystem>

#include "Pch.h"
#include "ShaderInformation.h"

namespace graphics
{

    unsigned int getGlslType(const std::filesystem::path &path);
    unsigned int compileShader(const std::filesystem::path &path);
    unsigned int compileShaderSource(unsigned int shaderType, const std::list<ShaderInformation> &data);
    std::vector<std::string> tokenise(const std::string &str, char delim=' ');

    class ShaderPreprocessor
    {
    public:
        explicit ShaderPreprocessor(const std::filesystem::path &path);
        void start();
        void orderByInclude();
        const std::list<ShaderInformation>& getSources() const { return mInformation; };

    protected:
        void preprocessInclude(std::string token, ShaderInformation& shaderData);
        void walk();
        void crash(const std::string &message) const;

        std::filesystem::path mInvokingPath;
        std::filesystem::path mCurrentPath;
        std::list<ShaderInformation> mInformation;
        std::set<std::filesystem::path> mAllPaths;
        std::unordered_map<std::string, int> mDefinitions;
    };
}
