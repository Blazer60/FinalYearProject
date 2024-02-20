/**
 * @file ShaderCompilation.h
 * @author Ryan Purse
 * @date 05/02/2024
 */


#pragma once

#include <filesystem>

#include "GraphicsDefinitions.h"
#include "Pch.h"
#include "ShaderInformation.h"

namespace graphics
{
    struct Macro
    {
        std::string symbol;
        int constant = 1;
    };

    unsigned int getGlslType(const std::filesystem::path &path);
    unsigned int compileShader(const std::filesystem::path &path, const std::vector<graphics::Macro>& macros);
    unsigned int compileShaderSource(unsigned int shaderType, const std::list<ShaderInformation> &data, const std::vector<Macro> &macros);
    std::vector<std::string> tokenise(const std::string &str, char delim=' ');

    class ShaderPreprocessor
    {
    public:
        explicit ShaderPreprocessor(const std::filesystem::path &path);
        void setupDefinitions(const std::vector<Macro>& macros);
        void start();
        void orderByInclude();
        const std::list<ShaderInformation>& getSources() const { return mInformation; }


    protected:
        void preprocessInclude(std::string token, ShaderInformation& shaderData, uint32_t depth);
        void walk(uint32_t depth);
        void crash(const std::string &message) const;

        std::filesystem::path mInvokingPath;
        std::filesystem::path mCurrentPath;
        std::list<ShaderInformation> mInformation;
        std::unordered_map<std::string, int> mDefinitions;
    };
}
