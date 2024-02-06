/**
 * @file ShaderCompilation.cpp
 * @author Ryan Purse
 * @date 05/02/2024
 */


#include "ShaderCompilation.h"

#include <fstream>
#include <queue>
#include <sstream>

#include "Logger.h"
#include "LoggerMacros.h"

namespace graphics
{
    unsigned int getGlslType(const std::filesystem::path& path)
    {
        const std::unordered_map<std::string, unsigned int> conversions {
            { ".vert", GL_VERTEX_SHADER },
            { ".frag", GL_FRAGMENT_SHADER },
            { ".comp", GL_COMPUTE_SHADER },
            { ".geom", GL_GEOMETRY_SHADER },
        };
        return conversions.at(path.extension().string());
    }

    unsigned int compileShader(const std::filesystem::path &path)
    {
        const unsigned int shaderType = getGlslType(path);
        ShaderPreprocessor preprocessor(path);
        preprocessor.start();
        preprocessor.orderByInclude();
        return compileShaderSource(shaderType, preprocessor.getSources());
    }

    std::vector<std::string> tokenise(const std::string& str, const char delim)
    {
        std::vector<std::string> out;
        auto startIt = str.begin();
        auto endIt = std::next(startIt);

        const auto addIfNonZeroLength = [&] {
            if (std::distance(startIt, endIt) > 1)
            {
                out.emplace_back(++startIt, endIt);
                startIt = endIt;
            }
        };

        while (endIt != str.end())
        {
            if (*endIt == delim)
                addIfNonZeroLength();
            ++endIt;
        }
        addIfNonZeroLength();

        return out;
    }

    ShaderPreprocessor::ShaderPreprocessor(const std::filesystem::path& path)
        : mInvokingPath(path)
    {
    }

    void ShaderPreprocessor::start()
    {
        walk(mInvokingPath);
    }

    void ShaderPreprocessor::orderByInclude()
    {
        std::set<ShaderData*> sorted;
        std::set<ShaderData*> toSort { &*mInformation.begin() };
        while (!toSort.empty())
        {
            const auto currentIt = toSort.begin();
            const ShaderData &currentShader = **currentIt;
            sorted.emplace(*currentIt);
            toSort.erase(currentIt);

            for (const auto &includePath : currentShader.includePaths)
            {
                const auto it = std::find_if(mInformation.begin(), mInformation.end(), [&includePath](const ShaderData &lhs) { return lhs.path == includePath; });
                mInformation.splice(mInformation.begin(), mInformation, it, std::next(it));
                if (sorted.count(&*it) == 0)
                    toSort.emplace(&*it);
            }
        }
    }

    void ShaderPreprocessor::preprocessInclude(std::string token, ShaderData& shaderData)
    {
        if (!shaderData.evaluationStack.top())
            return;

        if (token.empty() || token.size() < 2)
            crash(format::string("Expected a path after #include on line %", shaderData.lineCount));
        if (*token.begin() != '"' || *(std::prev(token.end())) != '"')
            crash(format::string("% must be surrounded by \"\" to be evaluated on line %.", token, shaderData.lineCount));

        const std::string relativePath(std::next(token.begin()), std::prev(token.end()));
        const std::filesystem::path absolutePath = absolute(shaderData.path.parent_path() / relativePath).lexically_normal();
        // todo: What should we do if the user trys to include a path twice?
        shaderData.includePaths.push_back(absolutePath);

        walk(absolutePath);
        shaderData.sourceBuffer << "#line " << shaderData.lineCount << "\n";
        mCurrentPath = shaderData.path;
    }

    void ShaderPreprocessor::preprocessIfdef(const std::string& token, ShaderData& shaderData)
    {
        shaderData.evaluationStack.emplace(mDefinitions[token] != 0 && shaderData.evaluationStack.top());
        shaderData.sourceBuffer << "#ifdef " << token << "\n";
    }

    void ShaderPreprocessor::preprocessEndif(ShaderData& shaderData)
    {
        shaderData.evaluationStack.pop();
        shaderData.sourceBuffer << "#endif\n";
    }

    void ShaderPreprocessor::preprocessElifdef(const std::string &token, ShaderData& shaderData)
    {
        const bool top = shaderData.evaluationStack.top();
        shaderData.evaluationStack.pop();
        shaderData.evaluationStack.emplace(mDefinitions[token] != 0 && !top && shaderData.evaluationStack.top());
        shaderData.sourceBuffer << "#elifdef " << token << "\n";
    }

    void ShaderPreprocessor::preprocessDelete(ShaderData& shaderData)
    {
        shaderData.sourceBuffer << "#line " << shaderData.lineCount << "\n";
    }

    void ShaderPreprocessor::preprocessDefine(const std::vector<std::string>& tokens, ShaderData &shaderData, const std::string &line)
    {
        if (tokens.size() == 2)
            mDefinitions[tokens[1]] = 1;
        else
            mDefinitions[tokens[1]] = std::stoi(tokens[2]);
        shaderData.sourceBuffer << line << "\n";
    }

    void ShaderPreprocessor::preprocessElse(ShaderData& shaderData)
    {
        const bool top = shaderData.evaluationStack.top();
        shaderData.evaluationStack.pop();
        shaderData.evaluationStack.emplace(!top && shaderData.evaluationStack.top());
        shaderData.sourceBuffer << "#else\n";
    }


    void ShaderPreprocessor::walk(const std::filesystem::path& path)
    {
        if (mAllPaths.count(path) > 0)  // todo: How do we also crash on a cyclic dependancy?
            return;  // We've already processed this file.

        std::ifstream file(path);
        if (file.bad() || file.fail())
            crash(format::string("Could not find file: %", path));

        mCurrentPath = path;

        std::string line;
        ShaderData shaderData;
        shaderData.evaluationStack.emplace(true);
        shaderData.path = path;
        while (std::getline(file, line))
        {
            const auto characterOffset = line.find_first_not_of(' ');
            if (characterOffset != std::string::npos && line[characterOffset] == '#')
            {
                std::vector<std::string> tokens = tokenise(line);

                // todo: Refactor this into map[](). Some of these functions should probs be members of the shader data at this point.
                // todo: pure pass the line too so it doesn't look weird for whoever wrote the shader.
                const std::string &command = tokens[0];
                if (command == "include")
                    preprocessInclude(tokens[1], shaderData);
                else if (command == "ifdef")
                    preprocessIfdef(tokens[1], shaderData);
                else if (command == "elifdef")
                    preprocessElifdef(tokens[1], shaderData);
                else if (command == "else")
                    preprocessElse(shaderData);
                else if (command == "endif")
                    preprocessEndif(shaderData);
                else if (command == "version" || command == "pragma")
                    preprocessDelete(shaderData);
                else if (command == "define")
                    preprocessDefine(tokens, shaderData, line);
            }
            else
                shaderData.sourceBuffer << line << "\n";
            ++shaderData.lineCount;
        }
        mInformation.emplace_front(std::move(shaderData));
        mAllPaths.emplace(path);
    }

    void ShaderPreprocessor::crash(const std::string& message) const
    {
        CRASH("Failed to compile shader %\nError while preprocessing file %.\n%", mInvokingPath, mCurrentPath, message);
    }

    unsigned int compileShaderSource(const unsigned int shaderType, const std::list<ShaderData> &data)
    {
        const char *prependShader = "#version 460 core\n";

        const unsigned shaderId = glCreateShader(shaderType);
        std::vector<std::string> sourceStrings;
        std::vector<const char *> dataPtrs;
        dataPtrs.reserve(data.size() + 1);
        dataPtrs.push_back(prependShader);
        for (const auto &shader : data)
            dataPtrs.push_back(sourceStrings.emplace_back(shader.sourceBuffer.str()).c_str());

        glShaderSource(shaderId, static_cast<int>(dataPtrs.size()), dataPtrs.data(), nullptr);
        glCompileShader(shaderId);

        int result { 0 };
        glGetShaderiv(shaderId, GL_COMPILE_STATUS, &result);  // OpenGL fails silently, so we need to check it ourselves.
        if (GL_TRUE == result)
            return shaderId;

        const std::unordered_map<unsigned int, std::string> shaderTypes {
            { GL_VERTEX_SHADER, "Vertex" },
            { GL_FRAGMENT_SHADER, "Fragment" },
            { GL_GEOMETRY_SHADER, "Geometry" },
            { GL_COMPUTE_SHADER, "Compute" },
        };

        // Failed to compile shader.
        int length;
        glGetShaderiv(shaderId, GL_INFO_LOG_LENGTH, &length);
        char *message = static_cast<char *>(alloca(length * sizeof(char)));  // No point doing a heap allocation.
        glGetShaderInfoLog(shaderId, length, &length, message);
        glDeleteShader(shaderId);

        // todo: This MUST tell us what shader we're from. We can do this with the line pragma. So I guess there may be a way of retreiving this?
        // Check the wiki.
        const std::string shaderNames = format::value(data.begin(), data.end(), [](const ShaderData &shaderData) { return shaderData.path.filename().string(); });
        CRASH("Failed to compile % shader %: \n%", shaderTypes.at(shaderType), shaderNames, message);
        return 0;
    }
}
