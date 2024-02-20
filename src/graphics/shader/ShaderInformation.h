/**
 * @file ShaderInformation.h
 * @author Ryan Purse
 * @date 07/02/2024
 */


#pragma once

#include <filesystem>
#include <sstream>
#include <stack>
#include <fstream>

#include "Pch.h"

namespace graphics
{
    class ShaderInformation
    {
    public:
        explicit ShaderInformation(std::filesystem::path path);
        void emitCurrentLine();
        void emitLineCount();
        void preprocessIfdef(const std::string& token, std::unordered_map<std::string, int> &definitions);
        void preprocessEndif();
        void preprocessElifdef(const std::string &token, std::unordered_map<std::string, int> &definitions);
        void preprocessDefine(const std::vector<std::string>& tokens, std::unordered_map<std::string, int> &definitions);
        void preprocessElse();
        std::filesystem::path preprocessInclude(const std::filesystem::path &includePath);

    public:
        std::filesystem::path path;
        std::vector<std::filesystem::path> includePaths;
        std::stringstream sourceBuffer;
        std::stack<bool> evaluationStack;
        std::string mCurrentLine;
        int lineCount { 2 };
        uint32_t depthFromSource;
    };
}
