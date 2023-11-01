/**
 * @file ResourcePool.h
 * @author Ryan Purse
 * @date 01/11/2023
 */


#pragma once

#include "Pch.h"
#include "Shader.h"
#include "Mesh.h"
#include "CommonLoader.h"

#include <assimp/DefaultLogger.hpp>
#include <assimp/Importer.hpp>
#include <assimp/Scene.h>
#include <assimp/postprocess.h>

namespace engine
{
/**
 * @author Ryan Purse
 * @date 01/11/2023
 */
    class ResourcePool
    {
    public:
        void clean();
        [[nodiscard]] std::shared_ptr<Shader> loadShader(const std::filesystem::path &vertexPath, const std::filesystem::path &fragmentPath);
        
        template<typename TVertex>
        [[nodiscard]] SharedMesh loadMesh(const std::filesystem::path &path);
        
        [[nodiscard]] std::shared_ptr<Texture> loadTexture(const std::filesystem::path &path);
    
    protected:
        // If we make sharedResource class, we can kill this when it only has a single use (here).
        std::unordered_map<std::string, std::shared_ptr<Shader>> mShaders;
        std::unordered_map<std::string, SharedMesh> mModels;
        std::unordered_map<std::string, std::shared_ptr<Texture>> mTextures;
    };
    
    
    template<typename TVertex>
    SharedMesh ResourcePool::loadMesh(const std::filesystem::path &path)
    {
        const std::string hashName = path.string() + std::to_string(typeid(TVertex).hash_code());
        if (auto it = mModels.find(hashName); it != mModels.end())
            return it->second;
        
        if (path.empty())
            return { };
        
        Assimp::Importer importer;
        const aiScene *scene = importer.ReadFile(
            path.string(),
            aiProcess_GlobalScale           |
            aiProcess_CalcTangentSpace      |
            aiProcess_Triangulate           |
            aiProcess_JoinIdenticalVertices |
            aiProcess_SortByPType);
        
        if (scene == nullptr)
        {
            WARN("Could not load model with path %\n%", path, importer.GetErrorString());
            return { };
        }
        
        
        SharedMesh sharedMesh = std::make_shared<std::vector<std::unique_ptr<SubMesh>>>();
        sharedMesh->reserve(scene->mNumMeshes);
        
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
                const glm::vec3 position = load::toVec3(mesh->mVertices[j]);
                const glm::vec2 uv = mesh->HasTextureCoords(0) ? load::toVec2(mesh->mTextureCoords[0][j]) : glm::vec2(0.f);
                const glm::vec3 normal = load::toVec3(mesh->mNormals[j]);
                const glm::vec3 tangent = mesh->HasTangentsAndBitangents() ? load::toVec3(mesh->mTangents[j]) : glm::vec3(0.f);
                // User defined conversion happens here.
                vertices.emplace_back(AssimpVertex { position, uv, normal, tangent });
            }
            
            if (!mesh->HasTextureCoords(0))
                WARN("Submesh % does not contain texture coordinates. (%)", i, path);
            else if (!mesh->HasTangentsAndBitangents())  // No uvs = no tangents.
                WARN("Submesh % does not have bi-/tangents. (%)", i, path);
            
            sharedMesh->emplace_back(std::make_unique<SubMesh>(vertices, indices));
        }
        
        // We're using assimp's logger so that the last message when collapsed is this.
        Assimp::DefaultLogger::get()->info("Load successful.");
        
        mModels[hashName] = sharedMesh;
        return sharedMesh;
    }
} // engine
