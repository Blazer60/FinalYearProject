/**
 * @file Loader.h
 * @author Ryan Purse
 * @date 16/12/2023
 */


#pragma once

#include "Pch.h"

#include <filesystem>
#include <glfw3.h>

#include "EngineState.h"
#include "Mesh.h"
#include "CommonLoader.h"
#include "ResourcePool.h"

#include <assimp/DefaultLogger.hpp>
#include <assimp/Importer.hpp>
#include <assimp/Scene.h>
#include <assimp/postprocess.h>
#include <yaml-cpp/node/node.h>

#include "Texture.h"

namespace engine
{
    namespace physics
    {
        struct MeshColliderBuffer;
    }

    class AudioSource;
    class Scene;
    class Actor;
}

namespace load
{
    /**
     * \brief Creates a physics mesh that can be used by mesh colliders.
     */
    std::shared_ptr<engine::physics::MeshColliderBuffer> physicsMesh(const std::filesystem::path &path);

    /**
     * \brief Creates an audio source that can be used by a sound component.
     */
    std::unique_ptr<engine::AudioSource> audio(const std::filesystem::path &path);

    /**
     * \brief Creates a scene from a .pcy file.
     */
    std::unique_ptr<engine::Scene> scene(const std::filesystem::path &path);

    /**
     * \brief Creates a shder that can be used by the renderer.
     * \param vertexPath The path to the vertex shader
     * \param fragmentPath The path to the fragment shader
     */
    std::shared_ptr<Shader> shader(const std::filesystem::path &vertexPath, const std::filesystem::path &fragmentPath);

    /**
     * @brief Creates a textures that can be used anywhere.
     * @param path - The path to the texture that you want to load.
     * @returns A texture object. ->id() is 0 if it failed to load the texture
     * (check the runtime logs for more info).
     */
    std::shared_ptr<Texture> texture(const std::filesystem::path &path);

    /**
     * \brief Creates a task bar icon that can shown by windows.
     */
    GLFWimage windowIcon(std::string_view path);

    /**
     * \brief Loads an actor into a scene.
     */
    void actor(const YAML::Node &actorNode, engine::Scene *scene);

    /**
     * \brief Creates a model that can be used by the renerer.
     * \tparam TVertex The type of vertex that you want the mesh to use.
     * \param path The path to the model.
     * \returns A shared mesh that contains multiple SubMeshes.
     */
    template<typename TVertex>
    SharedMesh model(const std::filesystem::path &path);

    /**
     * @brief Creates a single submesh with the first mesh found within the model loaded.
     * This function with crash if the submesh is not valid.
     * Use load::model if you want safety instead. Primitives are not cached.
     * @tparam TVertex - The type of vertex used to construct the primitive.
     * @param path - The path to the primitive.
     */
    template<typename TVertex>
    SubMesh primitive(const std::filesystem::path &path);

    std::shared_ptr<engine::UberLayer> materialLayer(const std::filesystem::path &path);
    std::shared_ptr<engine::UberMaterial> material(const std::filesystem::path &path);
}

// Implementations.

namespace load
{
    template<typename TVertex>
    SharedMesh model(const std::filesystem::path &path)
    {
        return engine::resourcePool->loadMesh<TVertex>(path);
    }

    template<typename TVertex>
    SubMesh primitive(const std::filesystem::path &path)
    {
        Assimp::Importer importer;
        const aiScene *scene = importer.ReadFile(
            path.string(),
            aiProcess_CalcTangentSpace      |
            aiProcess_Triangulate           |
            aiProcess_JoinIdenticalVertices |
            aiProcess_SortByPType);

        if (scene == nullptr)
            CRASH("Could not load model with path %\n%", path, importer.GetErrorString());

        if (scene->mNumMeshes < 1)
            CRASH("Primitive does not contain any meshes (%).", path);
        else if (scene->mNumMeshes > 1)
            WARN("Primitive has too many meshes. Only the first one will be used (%).", path);

        const aiMesh *mesh = scene->mMeshes[0];

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
            WARN("Primitive does not contain texture coordinates. (%)", path);
        else if (!mesh->HasTangentsAndBitangents())  // No uvs = no tangents.
            WARN("Primitive does not have bi-/tangents. (%)", path);

        return SubMesh(vertices, indices);
    }
}
