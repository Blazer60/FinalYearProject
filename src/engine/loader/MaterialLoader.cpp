/**
 * @file MtlLoader.cpp
 * @author Ryan Purse
 * @date 09/03/2022
 */


#include "MaterialLoader.h"

namespace load
{
    void parseMaterial(std::string_view path, const std::function<void(std::string&, MtlMaterialInformation&)> &callback)
    {
        MtlMaterialInformation currentMaterial {};
        std::string currentName     { "NULL" };
        
        /**
         * @brief Converts the current material into a new material. Ignores the first newmtl.
         */
        const auto newMaterial = [&](std::string_view arg) {
            static bool isFirst = true;
            if (isFirst)
            {
                currentName = std::string(arg);
                isFirst = false;
                return;
            }
            callback(currentName, currentMaterial);
            currentMaterial = MtlMaterialInformation();
            currentName     = std::string(arg);
        };
        
        const ArgumentList argumentList {
            {"#",        doNothing },
            {"d",        doNothing },
            {"Ke",       doNothing },
            {"Ni",       doNothing },
            {"map_Kd",   [&](std::string_view arg) { currentMaterial.mapKd = convertRelativePath(path, arg); } },
            {"map_Ke",   [&](std::string_view arg) { currentMaterial.mapKe = convertRelativePath(path, arg); } },
            {"map_Ns",   [&](std::string_view arg) { currentMaterial.mapNs = convertRelativePath(path, arg); } },
            {"map_Bump", [&](std::string_view arg) { currentMaterial.mapBump = convertRelativePath(path, arg); } },
            {"illum",    doNothing },
            {"newmtl",   newMaterial },
            {"Ns",       [&currentMaterial](std::string_view arg)  { currentMaterial.nS = std::stoi(std::string(arg)); } },
            {"Ka",       [&currentMaterial](std::string_view args) { currentMaterial.kA = createVec<3>(args); } },
            {"Kd",       [&currentMaterial](std::string_view args) { currentMaterial.kD = createVec<3>(args); } },
            {"Ks",       [&currentMaterial](std::string_view args) { currentMaterial.kS = createVec<3>(args); } },
        };
        
        parseFile(path, argumentList);
        newMaterial("NULL");  // Convert the last known material when we reach the end of the file.
    }
}


