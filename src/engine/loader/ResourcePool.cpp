/**
 * @file ResourcePool.cpp
 * @author Ryan Purse
 * @date 01/11/2023
 */


#include "ResourcePool.h"
#include <Statistics.h>
#include <FileLoader.h>

namespace engine
{
    template<typename T>
    void internalClean(std::unordered_map<std::string, std::shared_ptr<T>> &map)
    {
        std::vector<std::string> toDelete;
        for (auto &[hashName, value] : map)
        {
            if (value.use_count() <= 1)
                toDelete.push_back(hashName);
        }
        for (const std::string &hashName : toDelete)
        {
            MESSAGE("Cleaning up %", hashName);
            map.erase(hashName);
        }
    }

    template<typename>
    void internalClean(std::unordered_map<std::string, std::shared_ptr<UberLayer>> &map)
    {
        std::vector<std::string> toDelete;
        for (auto &[hashName, value] : map)
        {
            if (value.use_count() <= 1)
            {
                value->saveToDisk();
                toDelete.push_back(hashName);
            }
        }
        for (const std::string &hashName : toDelete)
        {
            MESSAGE("Cleaning up %", hashName);
            map.erase(hashName);
        }
    }

    void ResourcePool::clean()
    {
        internalClean(mShaders);
        internalClean(mModels);
        internalClean(mTextures);
        internalClean(mAudioBuffers);
        internalClean(mMeshColliders);
        internalClean(mMaterialLayers);
    }

    void ResourcePool::saveAllAssets()
    {
        for (auto [_, materialLayer] : mMaterialLayers)
            materialLayer->saveToDisk();
    }

    std::shared_ptr<Shader> ResourcePool::loadShader(
        const std::filesystem::path &vertexPath,
        const std::filesystem::path &fragmentPath)
    {
        const std::string hashName = vertexPath.string() + fragmentPath.string();
        if (auto it = mShaders.find(hashName); it != mShaders.end())
            return it->second;
        
        // Currently no error handling for incorrect path.
        auto resource = std::make_shared<Shader>(
            std::vector { vertexPath, fragmentPath },
            std::vector { graphics::Definition { "FRAGMENT_OUTPUT" } });  // ad-hoc fix since we'll be removing custom shaders soon.
        mShaders[hashName] = resource;
        return resource;
    }
    
    std::shared_ptr<Texture> ResourcePool::loadTexture(const std::filesystem::path &path)
    {
        const std::string hashName = path.string();
        if (auto it = mTextures.find(hashName); it != mTextures.end())
            return it->second;
        
        auto resource = std::make_shared<Texture>(path);
        mTextures[hashName] = resource;
        return resource;
    }

    std::shared_ptr<AudioBuffer> ResourcePool::loadAudioBuffer(const std::filesystem::path& path)
    {
        const std::string hashName = path.string();
        if (auto it = mAudioBuffers.find(hashName); it != mAudioBuffers.end())
            return it->second;

        auto resource = std::make_shared<AudioBuffer>(path);
        mAudioBuffers[hashName] = resource;
        return resource;
    }

    std::shared_ptr<physics::MeshColliderBuffer> ResourcePool::loadPhysicsMesh(const std::filesystem::path& path)
    {
        const std::string hashName = path.string();
        if (const auto it = mMeshColliders.find(hashName); it != mMeshColliders.end())
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

        auto meshColliderBuffer = std::make_shared<engine::physics::MeshColliderBuffer>();

        for (int i = 0; i < scene->mNumMeshes; ++i)
        {
            engine::physics::MeshDataBuffer meshDataBuffer;

            const aiMesh *mesh = scene->mMeshes[i];

            for (int j = 0; j < mesh->mNumFaces; ++j)
            {
                for (int k = 0; k < mesh->mFaces[j].mNumIndices; ++k)
                {
                    const int index = mesh->mFaces[j].mIndices[k];
                    meshDataBuffer.indices.emplace_back(index);
                }
            }

            meshDataBuffer.vertices.reserve(mesh->mNumVertices);
            for (int j = 0; j < mesh->mNumVertices; ++j)
            {
                const glm::vec3 position = load::toVec3(mesh->mVertices[j]);
                meshDataBuffer.vertices.emplace_back(position);
            }

            meshColliderBuffer->meshDataBuffers.push_back(std::move(meshDataBuffer));
        }

        // We're using assimp's logger so that the last message when collapsed is this.
        Assimp::DefaultLogger::get()->info("Load successful.");

        for (const physics::MeshDataBuffer &dataBuffer : meshColliderBuffer->meshDataBuffers)
        {
            btIndexedMesh indexedMesh;
            // The most annoying interface for a mesh.
            indexedMesh.m_numTriangles = static_cast<int>(dataBuffer.indices.size()) / 3;
            indexedMesh.m_triangleIndexBase = (const unsigned char*)dataBuffer.indices.data();
            indexedMesh.m_triangleIndexStride = 3 * sizeof(int);
            indexedMesh.m_numVertices = static_cast<int>(dataBuffer.vertices.size());
            indexedMesh.m_vertexBase = (const unsigned char*)dataBuffer.vertices.data();
            indexedMesh.m_vertexStride = sizeof(float) * 3;
            indexedMesh.m_indexType = PHY_INTEGER;
            meshColliderBuffer->indexedMeshes.push_back(indexedMesh);
        }

        for (btIndexedMesh indexedMesh : meshColliderBuffer->indexedMeshes)
            meshColliderBuffer->vertexArray.addIndexedMesh(indexedMesh, PHY_INTEGER);

        mMeshColliders[hashName] = meshColliderBuffer;
        return meshColliderBuffer;
    }

    std::shared_ptr<UberLayer> ResourcePool::loadMaterialLayer(const std::filesystem::path& path)
    {
        const std::string hashName = path.string();
        if (const auto it = mMaterialLayers.find(hashName); it != mMaterialLayers.end())
            return it->second;

        if (path.empty())
            return { };

        auto materialLayer = std::make_shared<UberLayer>(path);
        mMaterialLayers[hashName] = materialLayer;
        return materialLayer;
    }
}
