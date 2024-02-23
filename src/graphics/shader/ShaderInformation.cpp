/**
 * @file ShaderInformation.cpp
 * @author Ryan Purse
 * @date 07/02/2024
 */


#include "ShaderInformation.h"

#include <functional>

#include "Logger.h"
#include "LoggerMacros.h"
#include "StringManipulation.h"

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

    void ShaderInformation::preprocessIf(
        const std::vector<std::string>& tokens, const std::unordered_map<std::string, int> &definitions)
    {
        const std::string expression = join(std::next(tokens.begin()), tokens.end());
        if (expression.empty())
            CRASH("Expected expression");

        const std::vector<char> blackList { ' ', '(', ')' };
        if (expression.rfind("defined", 0) == 0)
        {
            constexpr int definedCharCount = 7;
            const std::string value = strip(
                std::next(expression.begin(), definedCharCount), expression.end(), blackList);

            evaluationStack.emplace(definitions.count(value) != 0 && evaluationStack.top());
        }
        else if (expression.rfind("!defined", 0) == 0)
        {
            constexpr int notDefinedCharCount = 8;
            const std::string value = strip(
                std::next(expression.begin(), notDefinedCharCount), expression.end(), blackList);

            evaluationStack.emplace(definitions.count(value) == 0 && evaluationStack.top());
        }
        else if (tokens.size() <= 2)
        {
            const int constant = isdigit(tokens[1][0]) != 0 ? std::stoi(tokens[1]) : definitions.at(tokens[1]);
            evaluationStack.emplace(constant != 0 && evaluationStack.top());
        }
        else
        {
            // Basic evaluation.
            const std::string& lhs = tokens[1];
            const std::string& operatorString = tokens[2];
            const std::string& rhs = tokens[3];

            const int lhsConstant = isdigit(lhs[0]) != 0 ? std::stoi(lhs) : definitions.at(lhs);
            const int rhsConstant = isdigit(rhs[0]) != 0 ? std::stoi(rhs) : definitions.at(rhs);

            const std::unordered_map<std::string, std::function<bool(int, int)>> opList
            {
                { "==", [](const int lhs, const int rhs) { return lhs == rhs; } },
                { "!=", [](const int lhs, const int rhs) { return lhs != rhs; } },
                { ">=", [](const int lhs, const int rhs) { return lhs >= rhs; } },
                { "<=", [](const int lhs, const int rhs) { return lhs <= rhs; } },
                { ">",  [](const int lhs, const int rhs) { return lhs > rhs; } },
                { "<",  [](const int lhs, const int rhs) { return lhs < rhs; } },
            };

            evaluationStack.emplace(opList.at(operatorString)(lhsConstant, rhsConstant) && evaluationStack.top());
        }

        emitCurrentLine();
    }

    void ShaderInformation::preprocessIfdef(const std::string& token, std::unordered_map<std::string, int> &definitions)
    {
        evaluationStack.emplace(definitions.count(token) != 0 && evaluationStack.top());
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
        evaluationStack.emplace(definitions.count(token) != 0 && !top && evaluationStack.top());
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
