/**
 * @file ShaderCompilation.h
 * @author Ryan Purse
 * @date 05/02/2024
 */


#pragma once

#include <filesystem>
#include <sstream>
#include <stack>
#include <unordered_set>

#include "Pch.h"

namespace graphics
{
    struct ShaderData
    {
        std::filesystem::path path;
        std::vector<std::filesystem::path> includePaths;
        std::stringstream sourceBuffer;
        std::stack<bool> evaluationStack;
        int lineCount { 2 };
    };

    struct PreprocessorInformation
    {
        std::filesystem::path invokingPath;
        std::unordered_map<std::string, int> definitions;
    };

    unsigned int getGlslType(const std::filesystem::path &path);
    unsigned int compileShader(const std::filesystem::path &path);

    unsigned int compileShaderSource(unsigned int shaderType, const std::list<ShaderData> &data);

    std::vector<std::string> tokenise(const std::string &str, char delim=' ');

    class ShaderPreprocessor
    {
    public:
        explicit ShaderPreprocessor(const std::filesystem::path &path);
        void start();

        void orderByInclude();

        const std::list<ShaderData>& getSources() const { return mInformation; };

    protected:
        void preprocessInclude(std::string token, ShaderData& shaderData);

        void preprocessIfdef(const std::string &token, ShaderData& shaderData);

        void preprocessEndif(ShaderData& shaderData);

        void preprocessElse(ShaderData& shaderData);

        void preprocessElifdef(const std::string &token, ShaderData& shaderData);

        void preprocessDelete(ShaderData& shaderData);

        void preprocessDefine(const std::vector<std::string> &tokens, ShaderData &shaderData, const std::string &line);

        void walk(const std::filesystem::path &path);
        void crash(const std::string &message) const;

        std::filesystem::path mInvokingPath;
        std::filesystem::path mCurrentPath;
        std::list<ShaderData> mInformation;
        std::set<std::filesystem::path> mAllPaths;
        std::unordered_map<std::string, int> mDefinitions;
    };
}
