/**
 * @file ShaderCompilation.cpp
 * @author Ryan Purse
 * @date 05/02/2024
 */


#include "ShaderCompilation.h"

#include <fstream>
#include <functional>
#include <queue>
#include <sstream>

#include "GraphicsDefinitions.h"
#include "Logger.h"
#include "LoggerMacros.h"
#include "StringManipulation.h"

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

    unsigned int compileShader(const std::filesystem::path &path, const std::vector<Definition>& macros)
    {
        const unsigned int shaderType = getGlslType(path);
        ShaderPreprocessor preprocessor(path);
        preprocessor.setupDefinitions(macros);
        preprocessor.start();
        preprocessor.orderByInclude();
        return compileShaderSource(shaderType, preprocessor.getSources(), macros);
    }

    ShaderPreprocessor::ShaderPreprocessor(const std::filesystem::path& path)
        : mInvokingPath(path)
    {
    }

    void ShaderPreprocessor::setupDefinitions(const std::vector<Definition>& definitions)
    {
        for (const auto & [symbol, constant] : definitions)
            mDefinitions[symbol] = constant;
    }

    void ShaderPreprocessor::start()
    {
        mCurrentPath = mInvokingPath;
        walk(0);
    }

    void ShaderPreprocessor::orderByInclude()
    {
        std::list toSort { &*mInformation.begin() };  // Breadth first so order matters here.
        while (!toSort.empty())
        {
            const auto currentIt = toSort.begin();
            const ShaderInformation &currentShader = **currentIt;
            toSort.erase(currentIt);

            for (const auto &includePath : currentShader.includePaths)
            {
                const auto it = std::find_if(mInformation.begin(), mInformation.end(),
                    [&includePath](const ShaderInformation &lhs) {
                        return lhs.path == includePath;
                });

                if (it != mInformation.begin())  // Move to the front of the list.
                    mInformation.splice(mInformation.begin(), mInformation, it, std::next(it));

                toSort.emplace_back(&*it);
            }
        }
    }

    void ShaderPreprocessor::preprocessInclude(std::string token, ShaderInformation& shaderData, const uint32_t depth)
    {
        if (token.empty() || token.size() < 2)
            crash(format::string("Expected a path after #include on line %", shaderData.lineCount));
        if (!(*token.begin() == '"' && *std::prev(token.end()) == '"') && !(*token.begin() == '<' && *std::prev(token.end()) == '>'))
            crash(format::string("% must be surrounded by \"\" or <> to be evaluated on line %.", token, shaderData.lineCount));
        const std::string relativePath(std::next(token.begin()), std::prev(token.end()));
        const std::filesystem::path absolutePath = std::filesystem::absolute(mCurrentPath.parent_path() / relativePath).lexically_normal();

        if (const auto newPath = shaderData.preprocessInclude(absolutePath); !newPath.empty())
        {
            mCurrentPath = newPath;
            walk(depth + 1);
        }
        mCurrentPath = shaderData.path;
    }

    void ShaderPreprocessor::walk(const uint32_t depth)
    {
        auto it = std::find_if(mInformation.begin(), mInformation.end(),
            [&](const ShaderInformation &shaderInformation) {
            return shaderInformation.path == mCurrentPath;
        });
        if (it != mInformation.end())
        {
            it->depthFromSource = glm::max(it->depthFromSource, depth);
            return;  // We've already processed this file or in the process of.
        }

        std::ifstream file(mCurrentPath);
        if (file.bad() || file.fail())
            crash(format::string("Could not find file: %", mCurrentPath));

        ShaderInformation &shaderData = mInformation.emplace_back(mCurrentPath);
        shaderData.depthFromSource = depth;
        std::vector<std::string> tokens;
        const std::unordered_map<std::string, std::function<void()>> commands
        {
            { "#include", [&] { preprocessInclude(tokens[1], shaderData, depth); } },
            { "#if",      [&] { shaderData.preprocessIf(tokens, mDefinitions); } },
            { "#ifdef",   [&] { shaderData.preprocessIfdef(tokens[1], mDefinitions); } },
            { "#elifdef", [&] { shaderData.preprocessElifdef(tokens[1], mDefinitions); } },
            { "#else",    [&] { shaderData.preprocessElse(); } },
            { "#endif",   [&] { shaderData.preprocessEndif(); } },
            { "#version", [&] { shaderData.emitLineCount(); } },
            { "#pragma",  [&] { shaderData.emitLineCount(); } },
            { "#define",  [&] { shaderData.preprocessDefine(tokens, mDefinitions); } },
            { "#line",    [&] { shaderData.emitCurrentLine(); } },
        };

        std::string line;
        while (std::getline(file, line))
        {
            shaderData.mCurrentLine = line;
            const auto characterOffset = line.find_first_not_of(' ');
            if (characterOffset != std::string::npos && line[characterOffset] == '#')
            {
                tokens = split(line);
                const std::string &command = tokens[0];
                commands.at(command)();
            }
            else
                shaderData.emitCurrentLine();
            ++shaderData.lineCount;
        }
    }

    void ShaderPreprocessor::crash(const std::string& message) const
    {
        CRASH("Failed to compile shader %\nError while preprocessing file %.\n%", mInvokingPath, mCurrentPath, message);
    }

    unsigned int compileShaderSource(const unsigned int shaderType, const std::list<ShaderInformation> &data, const std::vector<Definition> &macros)
    {
        std::stringstream prependShader;
        prependShader << "#version 460 core\n";
        for (const auto & [symbol, constant] : macros)
            prependShader << "#define " << symbol << " " << constant << "\n";
        const std::string prependShaderString = prependShader.str();

        const unsigned shaderId = glCreateShader(shaderType);
        std::vector<std::string> sourceStrings;
        std::vector<const char *> dataPtrs;
        dataPtrs.reserve(data.size() + 1);
        dataPtrs.push_back(prependShaderString.c_str());
        int shaderIndex = 0;
        for (const auto &shader : data)
        {
            const std::string shaderSource = format::string("#line 1 %\n%", shaderIndex, shader.sourceBuffer);
            dataPtrs.push_back(sourceStrings.emplace_back(shaderSource).c_str());
            ++shaderIndex;
        }

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
        int length = 0;
        glGetShaderiv(shaderId, GL_INFO_LOG_LENGTH, &length);
        char *message = static_cast<char *>(alloca(length * sizeof(char)));  // No point doing a heap allocation.
        glGetShaderInfoLog(shaderId, length, &length, message);
        glDeleteShader(shaderId);

        const std::string shaderNames = format::value(data.begin(), data.end(), [](const ShaderInformation &shaderData) { return shaderData.path.filename().string(); });
        CRASH("Failed to compile % shader %: \n%", shaderTypes.at(shaderType), shaderNames, message);
        return 0;
    }
}
