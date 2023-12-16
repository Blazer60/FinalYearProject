/**
 * @file Loader.cpp
 * @author Ryan Purse
 * @date 16/12/2023
 */


#include "Loader.h"

#include <fstream>
#include <stb_image.h>

#include "EngineState.h"
#include "ResourcePool.h"
#include "Scene.h"

namespace load
{
    std::shared_ptr<engine::physics::MeshColliderBuffer> physicsMesh(const std::filesystem::path& path)
    {
        return engine::resourcePool->loadPhysicsMesh(path);
    }

    std::unique_ptr<engine::AudioSource> audio(const std::filesystem::path& path)
    {
        return std::make_unique<engine::AudioSource>(engine::resourcePool->loadAudioBuffer(path));
    }

    std::unique_ptr<engine::Scene> scene(const std::filesystem::path& path)
    {
        if (!exists(path))
        {
            WARN("Path to scene does not exist. No scene will be loaded. (%)", path);
            return std::make_unique<engine::Scene>();
        }

        std::ifstream stream(path);
        std::stringstream stringStream;
        stringStream << stream.rdbuf();
        stream.close();

        YAML::Node data = YAML::Load(stringStream.str());
        if (!data["Scene"])
        {
            WARN("File does not contain a scene: %", path);
            return std::make_unique<engine::Scene>();
        }

        const auto sceneName = data["Scene"].as<std::string>();
        MESSAGE("loading scene: %", sceneName);

        auto scene = engine::serializer->loadScene(data);

        for (const YAML::Node &actorNode : data["Actors"])
            load::actor(actorNode, scene.get());

        // Linking all of the children to their parents.
        for (Ref<engine::Actor> actor : scene->mToAdd)
        {
            for (engine::UUID childId : actor->getChildren())
            {
                auto child = scene->getActor(childId);
                child->mParent = actor.get();
            }
        }

        return scene;
    }

    [[nodiscard]] std::shared_ptr<Shader> shader(
        const std::filesystem::path &vertexPath,
        const std::filesystem::path &fragmentPath)
    {
        return engine::resourcePool->loadShader(vertexPath, fragmentPath);
    }

    std::shared_ptr<Texture> texture(const std::filesystem::path &path)
    {
        return engine::resourcePool->loadTexture(path);
    }

    GLFWimage windowIcon(std::string_view path)
    {
        stbi_set_flip_vertically_on_load(false);
        std::filesystem::path systemPath(path);
        if (!std::filesystem::exists(systemPath))
        {
            WARN("File % does not exist. The Window Icon's data will be null.", systemPath);
            return GLFWimage { };
        }

        int width;
        int height;
        int colourChannels;
        // Forcing 4 components so that is always aligns with opengl's internal format.
        unsigned char *bytes = stbi_load(systemPath.string().c_str(), &width, &height, &colourChannels, 4);

        GLFWimage icon;
        icon.pixels = bytes;
        icon.width = width;
        icon.height = height;

        return icon;
    }

    void actor(const YAML::Node &actorNode, engine::Scene *scene)
    {
        auto actor = engine::serializer->loadActor(actorNode, scene);
        actor->mName = actorNode["Name"].as<std::string>();
        actor->mId = actorNode["UUID"].as<engine::UUID>();
        actor->position = actorNode["position"].as<glm::vec3>();
        actor->rotation = actorNode["rotation"].as<glm::quat>();
        actor->scale = actorNode["scale"].as<glm::vec3>();
        actor->mChildren = actorNode["Children"].as<std::vector<engine::UUID>>();

        for (auto &componentNode : actorNode["Components"])
            engine::serializer->loadComponent(componentNode, actor);
    }
}
