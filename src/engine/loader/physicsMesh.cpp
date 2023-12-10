/**
 * @file physicsMesh.cpp
 * @author Ryan Purse
 * @date 10/12/2023
 */


#include "physicsMesh.h"

#include <assimp/DefaultLogger.hpp>
#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <assimp/Scene.h>
#include <BulletCollision/CollisionShapes/btBvhTriangleMeshShape.h>

#include "CommonLoader.h"
#include "Logger.h"
#include "LoggerMacros.h"

namespace load
{
    std::shared_ptr<engine::physics::MeshColliderBuffer> physicsMesh(const std::filesystem::path& path)
    {
        if (path.empty())
            return { };

        Assimp::Importer importer;
        const aiScene *scene = importer.ReadFile(
            path.string(),
            aiProcess_GlobalScale           |
            aiProcess_Triangulate           |
            aiProcess_JoinIdenticalVertices |
            aiProcess_OptimizeMeshes        |
            aiProcess_OptimizeGraph         );

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
            int localMaxIndex = 0;

            for (int j = 0; j < mesh->mNumFaces; ++j)
            {
                for (int k = 0; k < mesh->mFaces[j].mNumIndices; ++k)
                {
                    const int index = mesh->mFaces[j].mIndices[k];
                    localMaxIndex = glm::max(index, localMaxIndex);
                    meshDataBuffer.indices.emplace_back(index);
                }
            }

            meshDataBuffer.vertices.reserve(mesh->mNumVertices);
            for (int j = 0; j < mesh->mNumVertices; ++j)
            {
                const glm::vec3 position = toVec3(mesh->mVertices[j]);
                meshDataBuffer.vertices.emplace_back(position);
            }

            meshColliderBuffer->meshDataBuffers.push_back(std::move(meshDataBuffer));
        }

        // We're using assimp's logger so that the last message when collapsed is this.
        Assimp::DefaultLogger::get()->info("Load successful.");

        for (const engine::physics::MeshDataBuffer &dataBuffer : meshColliderBuffer->meshDataBuffers)
        {
            btIndexedMesh indexedMesh;
            indexedMesh.m_numTriangles = dataBuffer.indices.size() / 3;
            indexedMesh.m_triangleIndexBase = reinterpret_cast<const unsigned char*>(dataBuffer.indices.data());
            indexedMesh.m_triangleIndexStride = sizeof(int);
            indexedMesh.m_numVertices = dataBuffer.vertices.size();
            indexedMesh.m_vertexBase = reinterpret_cast<const unsigned char*>(dataBuffer.vertices.data());
            indexedMesh.m_vertexStride = sizeof(float) * 3;
            meshColliderBuffer->indexedMeshes.push_back(std::move(indexedMesh));
        }

        for (btIndexedMesh indexedMesh : meshColliderBuffer->indexedMeshes)
            meshColliderBuffer->vertexArray.addIndexedMesh(indexedMesh);

        return meshColliderBuffer;
    }
}
