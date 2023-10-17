/**
 * @file SceneLoader.cpp
 * @author Ryan Purse
 * @date 15/10/2023
 */


#include <fstream>
#include "SceneLoader.h"
#include "Scene.h"

#include <yaml-cpp/yaml.h>

namespace YAML
{
    template<>
    struct convert<glm::vec3>
    {
        static Node encode(const glm::vec3 &rhs)
        {
            Node node;
            node.push_back(rhs.x);
            node.push_back(rhs.y);
            node.push_back(rhs.z);
            return node;
        }
        
        static bool decode(const Node& node, glm::vec3 &rhs)
        {
            if (!node.IsSequence() || node.size() != 3)
                return false;
            
            rhs.x = node[0].as<float>();
            rhs.y = node[1].as<float>();
            rhs.z = node[2].as<float>();
            return true;
        }
    };
    
    template<>
    struct convert<glm::quat>
    {
        static Node encode(const glm::quat &rhs)
        {
            Node node;
            node.push_back(rhs.w);
            node.push_back(rhs.x);
            node.push_back(rhs.y);
            node.push_back(rhs.z);
            return node;
        }
        
        static bool decode(const Node& node, glm::quat &rhs)
        {
            if (!node.IsSequence() || node.size() != 4)
                return false;
            
            rhs.w = node[0].as<float>();
            rhs.x = node[1].as<float>();
            rhs.y = node[2].as<float>();
            rhs.z = node[3].as<float>();
            return true;
        }
    };
}

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
    }
    
    void actor(const YAML::Node &actorNode, engine::Scene *scene)
    {
        const auto name = actorNode["Actor"].as<std::string>() + " (Clone)";
        auto actor = scene->spawnActor<engine::Actor>(name);
        actor->position = actorNode["position"].as<glm::vec3>();
        actor->rotation = actorNode["rotation"].as<glm::quat>();
        actor->scale = actorNode["scale"].as<glm::vec3>();
        
        for (auto &childNode : actorNode["Children"])
            load::actor(childNode, actor);
    }
    
    void actor(const YAML::Node &actorNode, Ref<engine::Actor> parent)
    {
        const auto name = actorNode["Actor"].as<std::string>() + " (Clone)";
        Ref<engine::Actor> actor = parent->addChildActor(makeResource<engine::Actor>(name));
        actor->position = actorNode["position"].as<glm::vec3>();
        actor->rotation = actorNode["rotation"].as<glm::quat>();
        actor->scale = actorNode["scale"].as<glm::vec3>();
        
        for (auto &childNode : actorNode["Children"])
            load::actor(childNode, actor);
        
    }
}
