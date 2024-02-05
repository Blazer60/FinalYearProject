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
        std::list<ShaderData> shaderData;
        std::deque<std::filesystem::path> disassembledShaders { path };

        while (!disassembledShaders.empty())
        {
            const std::filesystem::path currentShader = disassembledShaders.front();
            disassembledShaders.pop_front();

            const auto data = passShader(currentShader);
            shaderData.push_front(data);
            for (const auto &includePath : data.includePaths)
            {
                const auto it = std::find_if(shaderData.begin(), shaderData.end(), [&includePath](const ShaderData &lhs) { return lhs.path == includePath; });
                if (it == shaderData.end())
                    disassembledShaders.push_back(includePath);
                else
                    shaderData.splice(shaderData.begin(), shaderData, it, std::next(it));
            }
        }

        return compileShaderSource(shaderType, shaderData);
    }

    unsigned int compileShaderSource(const unsigned int shaderType, const std::list<ShaderData> &data)
    {
        const char *prependShader = "#version 460 core\n";

        const unsigned shaderId = glCreateShader(shaderType);
        std::vector<const char *> dataPtrs;
        dataPtrs.reserve(data.size() + 1);
        dataPtrs.push_back(prependShader);
        for (const auto & [_, __, source] : data)
            dataPtrs.push_back(source.c_str());

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

        const std::string shaderNames = format::value(data.begin(), data.end(), [](const ShaderData &shaderData) { return shaderData.path.filename().string(); });
        ERROR("Failed to compile % shader %: \n%", shaderTypes.at(shaderType), shaderNames, message);
        return 0;
    }

    ShaderData passShader(const std::filesystem::path& path)
    {
        std::ifstream file(path);
        if (file.bad() || file.fail())
        {
            ERROR("Failed to open file at: %", path);
            return { };
        }

        ShaderData data;
        std::stringstream buffer;
        std::string line;

        int lineCount = 2;
        while (std::getline(file, line))
        {
            if (line.find("#include ") != std::string::npos)
            {
                auto startOffset = line.find_first_of('"');
                const auto endOffset = line.find_last_of('"');
                if (startOffset == std::string::npos || endOffset == std::string::npos)
                {
                    ERROR("Invalid #include statement on line %. The path must be surrounded by \"\". \n Path: %", lineCount, path);
                }
                else
                {
                    startOffset += 1;
                    const std::string relativePath(&line[startOffset], endOffset - startOffset);
                    const std::filesystem::path absolutePath = (std::filesystem::absolute(path.parent_path() / relativePath)).lexically_normal();
                    data.includePaths.push_back(absolutePath);
                }

                buffer << "#line " << lineCount << '\n';  // So that debug messages are still point to the correct place.
            }
            else if (line.find("#version ") != std::string::npos)
            {
                buffer << "#line " << lineCount << '\n';  // So that debug messages are still point to the correct place.
            }
            else
            {
                buffer << line << '\n';
            }

            ++lineCount;
        }

        data.path = path;
        data.source = buffer.str();

        return data;
    }
}
