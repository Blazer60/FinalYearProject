/**
 * @file SceneLoader.cpp
 * @author Ryan Purse
 * @date 15/10/2023
 */


#include <fstream>
#include "SceneLoader.h"
#include "Scene.h"
#include "AssimpLoader.h"
#include "FileLoader.h"
#include "EngineState.h"
#include "Core.h"

#include <yaml-cpp/yaml.h>


namespace load
{
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
