/**
 * @file ModelLoader.h
 * @author Ryan Purse
 * @date 12/03/2022
 */


#pragma once

#include "Pch.h"
#include "Vertices.h"
#include "CommonLoader.h"
#include "MaterialLoader.h"
#include "RawMesh.h"

#include <utility>

namespace load
{
    /**
     * @brief Stores the raw data loaded by the object file.
     */
    class ObjectFile;
    
    template<typename TVertex, typename TMaterial>
    std::pair<RawMeshes<TVertex>, Materials<TMaterial>> parseObject(std::string_view path);

    /**
     * @brief Loads a model with the correct Vertex type and material Type specified.
     * @tparam TVertex - The type of vertex that you want to use.
     * @tparam TMaterial - The type of material that you want to use.
     * @param path - The path to the model.
     * @returns Information that can be used to render the model.
     */
    template<typename TVertex, typename TMaterial=NoMaterial>
    Model<TVertex, TMaterial> model(std::string_view path)
    {
        const auto [meshes, materials] = parseObject<TVertex, TMaterial>(path);
        
        Model<TVertex, TMaterial> model;
        
        for (const auto &[name, rawMesh] : meshes)
        {
            TMaterial material = materials.count(name) > 0 ? materials.at(name) : TMaterial();
            model.mesh.emplace_back(rawMesh.toSharedSubMesh());
            model.materials.emplace_back(std::move(material));
        }
        
        return std::move(model);
    }
    
    template<typename TVertex, typename TMaterial>
    std::pair<RawMeshes<TVertex>, Materials<TMaterial>> parseObject(std::string_view path)
    {
        ObjectFile objectFile;
        RawMeshes<TVertex> meshes;
        std::string materialGroupName { "Null" };
    
        Materials<TMaterial> materials;
    
        const auto generateFace = [&](std::string_view arg) {
            meshes[materialGroupName].createVertices(arg, [&](std::string_view arg) {
                return objectFile.createVertex(arg);
            });
        };
    
        const auto loadMaterial = [&](std::string_view arg) {
            if (typeid(TMaterial) == typeid(NoMaterial))  // todo: can this be made constexpr?
                return;  // We don't want to load a material if no material is set.
            materials = load::material<TMaterial>(convertRelativePath(path, arg));
        };
    
        const ArgumentList argumentList {
            {"#",      doNothing },
            {"o",      doNothing },
            {"s",      doNothing },
            {"g",      doNothing },
            {"mtllib", loadMaterial },
            {"usemtl", [&materialGroupName](std::string_view arg) { materialGroupName = std::string(arg); } },
            {"v",      [&objectFile](std::string_view arg) { objectFile.positions.emplace_back(createVec<3>(arg));  } },
            {"vt",     [&objectFile](std::string_view arg) { objectFile.uvs.emplace_back(createVec<2>(arg));        } },
            {"vn",     [&objectFile](std::string_view arg) { objectFile.normals.emplace_back(createVec<3>(arg));    } },
            {"f",      generateFace },
        };
    
        parseFile(path, argumentList);
        return { std::move(meshes), std::move(materials) };
    }
    
    class ObjectFile
    {
    public:
        ObjVertex createVertex(std::string_view arg);
        
        std::vector<glm::vec3> positions;
        std::vector<glm::vec2> uvs;
        std::vector<glm::vec3> normals;
    };
}



