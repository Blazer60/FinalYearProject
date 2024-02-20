/**
 * @file ShaderInformation.cpp
 * @author Ryan Purse
 * @date 07/02/2024
 */


#include "ShaderInformation.h"

namespace graphics
{
    ShaderInformation::ShaderInformation(std::filesystem::path path)
        : path(std::move(path))
    {
        evaluationStack.emplace(true);
    }

    void ShaderInformation::emitCurrentLine()
    {
        sourceBuffer << mCurrentLine << "\n";
    }

    void ShaderInformation::emitLineCount()
    {
        sourceBuffer << "#line " << lineCount << "\n";
    }

    void ShaderInformation::preprocessIfdef(const std::string& token, std::unordered_map<std::string, int> &definitions)
    {
        evaluationStack.emplace(definitions[token] != 0 && evaluationStack.top());
        emitCurrentLine();
    }

    void ShaderInformation::preprocessEndif()
    {
        evaluationStack.pop();
        emitCurrentLine();
    }

    void ShaderInformation::preprocessElifdef(const std::string &token, std::unordered_map<std::string, int> &definitions)
    {
        const bool top = evaluationStack.top();
        evaluationStack.pop();
        evaluationStack.emplace(definitions[token] != 0 && !top && evaluationStack.top());
        emitCurrentLine();
    }

    void ShaderInformation::preprocessDefine(const std::vector<std::string>& tokens, std::unordered_map<std::string, int> &definitions)
    {
        emitCurrentLine();
        if (!evaluationStack.top())
            return;

        if (tokens.size() == 2)
            definitions[tokens[1]] = 1;
        else
            definitions[tokens[1]] = std::stoi(tokens[2]);
    }

    void ShaderInformation::preprocessElse()
    {
        const bool top = evaluationStack.top();
        evaluationStack.pop();
        evaluationStack.emplace(!top && evaluationStack.top());
        emitCurrentLine();
    }

    std::filesystem::path ShaderInformation::preprocessInclude(const std::filesystem::path &includePath)
    {
        emitLineCount();
        if (!evaluationStack.top())
            return "";

        if (const auto it = std::find(includePaths.begin(), includePaths.end(), includePath); it != includePaths.end())
            return "";

        includePaths.push_back(includePath);
        return includePath;
    }
}
