/**
 * @file AssimpLoader.h
 * @author Ryan Purse
 * @date 22/08/2023
 */


#pragma once

#include "Pch.h"
#include "Vertices.h"
#include "Mesh.h"
#include "assimp/DefaultLogger.hpp"

#include <assimp/Importer.hpp>
#include <assimp/Scene.h>
#include <assimp/postprocess.h>

namespace load
{
    glm::vec3 toVec3(const aiVector3D &v);
    glm::vec2 toVec2(const aiVector2D &v);
    glm::vec2 toVec2(const aiVector3D &v);
    
    template<typename TVertex>
    SharedMesh model(std::string_view path);
    
    template<typename TVertex>
    SharedMesh model(std::string_view path)
    {
        Assimp::Importer importer;
        const aiScene *scene = importer.ReadFile(
            path.data(),
            aiProcess_CalcTangentSpace      |
            aiProcess_Triangulate           |
            aiProcess_JoinIdenticalVertices |
            aiProcess_SortByPType);
        
        if (scene == nullptr)
        {
            WARN("Could not load model with path %\n%", path, importer.GetErrorString());
            return { };
        }
        
        
        SharedMesh sharedMesh;
        sharedMesh.reserve(scene->mNumMeshes);
        
        for (int i = 0; i < scene->mNumMeshes; ++i)
        {
            const aiMesh *mesh = scene->mMeshes[i];
            
            std::vector<uint32_t> indices;
            indices.reserve(mesh->mNumFaces * 3);
            for (int j = 0; j < mesh->mNumFaces; ++j)
            {
                for (int k = 0; k < mesh->mFaces[j].mNumIndices; ++k)
                    indices.emplace_back(mesh->mFaces[j].mIndices[k]);
            }
            
            std::vector<TVertex> vertices;
            vertices.reserve(mesh->mNumVertices);
            for (int j = 0; j < mesh->mNumVertices; ++j)
            {
                const glm::vec3 position = toVec3(mesh->mVertices[j]);
                const glm::vec2 uv = mesh->HasTextureCoords(0) ? toVec2(mesh->mTextureCoords[0][j]) : glm::vec2(0.f);
                const glm::vec3 normal = toVec3(mesh->mNormals[j]);
                const glm::vec3 tangent = mesh->HasTangentsAndBitangents() ? toVec3(mesh->mTangents[j]) : glm::vec3(0.f);
                // User defined conversion happens here.
                vertices.emplace_back(AssimpVertex { position, uv, normal, tangent });
            }
            
            if (!mesh->HasTextureCoords(0))
                WARN("Submesh % does not contain texture coordinates. (%)", i, path);
            else if (!mesh->HasTangentsAndBitangents())  // No uvs = no tangents.
                WARN("Submesh % does not have bi-/tangents. (%)", i, path);
            
            sharedMesh.emplace_back(std::make_shared<SubMesh>(vertices, indices));
        }
        
        // We're using assimp's logger so that the last message when collapsed is this.
        Assimp::DefaultLogger::get()->info("Load successful.");
        return std::move(sharedMesh);
    }
}
