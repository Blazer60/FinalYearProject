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
    void scene(const std::filesystem::path &path, engine::Scene *scene)
    {
        if (!std::filesystem::exists(path))
        {
            WARN("Path to scene does not exist. No scene will be loaded. (%)", path);
            return;
        }
        
        std::ifstream stream(path);
        std::stringstream stringStream;
        stringStream << stream.rdbuf();
        stream.close();
        
        YAML::Node data = YAML::Load(stringStream.str());
        if (!data["Scene"])
        {
            WARN("File does not contain a scene: %", path);
            return;
        }
        
        const auto sceneName = data["Scene"].as<std::string>();
        MESSAGE("loading scene: %", sceneName);
        
        for (const YAML::Node &actorNode : data["Actors"])
            load::actor(actorNode, scene);

        // Linking all of the children to their parents.
        for (Ref<engine::Actor> actor : scene->mToAdd)
        {
            for (engine::UUID childId : actor->getChildren())
            {
                auto child = scene->getActor(childId);
                child->mParent = actor.get();
            }
        }
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
    
    void actor(const YAML::Node &actorNode, Ref<engine::Actor> parent)
    {
        const auto name = actorNode["Actor"].as<std::string>();
        Ref<engine::Actor> actor = parent->addChildActor(makeResource<engine::Actor>(name));
        actor->position = actorNode["position"].as<glm::vec3>();
        actor->rotation = actorNode["rotation"].as<glm::quat>();
        actor->scale = actorNode["scale"].as<glm::vec3>();
        
        for (auto &childNode : actorNode["Children"])
            load::actor(childNode, actor);
        
        for (auto &componentNode : actorNode["Components"])
            engine::serializer->loadComponent(componentNode, actor);
    }
}
