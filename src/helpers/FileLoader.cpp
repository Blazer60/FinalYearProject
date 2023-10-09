/**
 * @file FileLoader.cpp
 * @author Ryan Purse
 * @date 08/10/2023
 */


#include "FileLoader.h"
#include <Statistics.h>
#include <filesystem>

namespace file
{
    std::filesystem::path searchPaths[] {
        "", "..", "..\\.."
    };
    
    namespace data
    {
        std::filesystem::path resourcePath = "";
        std::filesystem::path modelPath = "";
        std::filesystem::path shaderPath = "";
        std::filesystem::path texturePath = "";
    };
    
    bool findModelFolder()
    {
        auto path = data::resourcePath / "models";
        if (exists(path))
        {
            data::modelPath = path;
            return true;
        }
        
        WARN("Could not find the model folder in %", data::resourcePath.string());
        return false;
    }
    
    bool findShaderFolder()
    {
        auto path = data::resourcePath / "shaders";
        if (exists(path))
        {
            data::shaderPath = path;
            return true;
        }
        
        WARN("Could not find the shader folder in %", data::resourcePath.string());
        return false;
    }
    
    bool findTextureFolder()
    {
        auto path = data::resourcePath / "textures";
        if (exists(path))
        {
            data::texturePath = path;
            return true;
        }
        
        WARN("Could not find the textures folder in %", data::resourcePath.string());
        return false;
    }
    
    bool findResourceFolder()
    {
        auto currentWorkingDirectory = std::filesystem::current_path();
        MESSAGE("cwd: %", currentWorkingDirectory.string());
        for (const auto &relativePaths : searchPaths)
        {
            auto path = currentWorkingDirectory / relativePaths / "resources";
            MESSAGE("Trying Path: %", path.string());
            if (exists(path))
            {
                MESSAGE("Found directory: %", path.string());
                data::resourcePath = path;
                bool foundAll = true;
                foundAll |= findModelFolder();
                foundAll |= findShaderFolder();
                foundAll |= findTextureFolder();
                
                if (foundAll)
                    MESSAGE("Found all paths to resources.");
                return foundAll;
            }
        }
        
        WARN("Could not find the resource folder. This may cause errors in the rest of the code.");
        return false;
    }
    
    std::filesystem::path resourcePath()
    {
        return data::resourcePath;
    }
    
    std::filesystem::path modelPath()
    {
        return data::modelPath;
    }
    
    std::filesystem::path shaderPath()
    {
        return data::shaderPath;
    }
    
    std::filesystem::path texturePath()
    {
        return data::texturePath;
    }
}

