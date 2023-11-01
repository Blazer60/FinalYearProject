/**
 * @file CommonLoader.h
 * @author Ryan Purse
 * @date 23/02/2022
 */


#pragma once

#include "Pch.h"
#include "Vertices.h"
#include "Mesh.h"
#include "Materials.h"

#include <glm.hpp>
#include <functional>
#include <charconv>
#include <filesystem>

namespace load
{
    glm::vec3 toVec3(const aiVector3D &v);
    glm::vec2 toVec2(const aiVector2D &v);
    glm::vec2 toVec2(const aiVector3D &v);
    
    template<typename TMaterial>
    struct Model
    {
        SharedMesh mesh;
        std::vector<std::shared_ptr<TMaterial>> materials;
    };
    
    /**
     * @brief Explicit definition to tell loaders that is should not load material files.
     */
    struct NoMaterial
    {
        NoMaterial() = default;
        explicit NoMaterial(const MtlMaterialInformation &mtl) {};
    };
    
    /**
      * @brief Argument list that gets passed to parseFile() so that it knows how to interpret each line in the file.
      */
    typedef std::unordered_map<std::string_view, std::function<void(std::string_view)>> ArgumentList;

    /**
     * @brief Opens and file and parses the file based on the argument list provided. Doesn't do anything by itself.
     * @param path - The path to the file that you want to open.
     * @param argumentList - The list of functions that you want to be called based on keywords.
     */
    void parseFile(std::string_view path, const ArgumentList &argumentList);

    /**
     * @brief Splits a string view based on the deliminator.
     * @param args - The string that you want to split
     * @param delim - The delim that you want to pass.
     * @returns A split string by delim.
     */
    std::vector<std::string> split(std::string_view args, char delim=' ');

    /**
     * @brief Doesn't do anythinig. Placeholder for when you know a keyword doesn't do anything.
     * @param _ - A dud.
     */
    void doNothing(std::string_view _);

    /**
     * @brief Attempts to convert a relative file path from a base file into a relative path from the .exe.
     * @param baseFile - The file that the relative path belongs to.
     * @param relativeFile - The relative path that you want to convert.
     * @returns A relative path from the .exe.
     */
    std::string convertRelativePath(std::string_view baseFile, std::string_view relativeFile);
    
    template<uint32_t count>
    glm::vec<count, float, glm::defaultp> createVec(std::string_view arg)
    {
        const char* nextNumber = arg.data();
        glm::vec<count, float, glm::defaultp> vec(0.f);
        for (int i = 0; i < count; ++i)
        {
            // Pointer to the end of the last number, Error code.
            auto [pointer, ec] = std::from_chars(nextNumber, nextNumber + arg.size(), vec[i]);
            if (ec != std::errc())  // An error was produced by the arg list.
                LOG_MAJOR("Invalid arg (" + std::string(arg) + ") for vector construction");
            
            nextNumber = ++pointer;  // There is always a space between numbers.
        }
        
        return vec;
    }
}

