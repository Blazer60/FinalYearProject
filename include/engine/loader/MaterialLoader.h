/**
 * @file MtlLoader.h
 * @author Ryan Purse
 * @date 09/03/2022
 */


#pragma once

#include "Pch.h"
#include "CommonLoader.h"

namespace load
{
    typedef std::function<void(std::string&, MtlMaterialInformation &)> generateMaterialSignature;
    
    /**
     * @brief Goes through a .mtl file processing its contents.
     * @param path - The path to the file you want to process.
     * @param callback - How you want to handle creating materials.
     */
    void parseMaterial(std::string_view path, const generateMaterialSignature &callback);
    
    template<typename TMaterial>
    using Materials = std::unordered_map<std::string, TMaterial>;
    
    /**
     * @brief Loads a material from the specified path.
     * @tparam TMaterial - The type of material that you want to load.
     * @param path - The path to the .mtl file.
     * @returns All materials within said file.
     */
    template<typename TMaterial>
    Materials<TMaterial> material(std::string_view path)
    {
        Materials<TMaterial> out;
    
        parseMaterial(path, [&](const std::string &materialName, const MtlMaterialInformation &mtlMaterial) {
            TMaterial material;
            material.onLoadMtlFile(mtlMaterial);
            out.emplace(materialName, std::move(material));
        });
        
        return std::move(out);
    }
}


