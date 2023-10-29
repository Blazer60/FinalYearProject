/**
 * @file SceneSerializer.cpp
 * @author Ryan Purse
 * @date 13/10/2023
 */


#include "Serializer.h"
#include "Scene.h"
#include "Actor.h"
#include "Component.h"
#include "EngineState.h"

#include <yaml-cpp/yaml.h>
#include <fstream>

YAML::Emitter &operator<<(YAML::Emitter &out, const glm::vec2 &v)
{
    out << YAML::Flow;
    out << YAML::BeginSeq << v.x << v.y << YAML::EndSeq;
    return out;
}

YAML::Emitter &operator<<(YAML::Emitter &out, const glm::vec3 &v)
{
    out << YAML::Flow;
    out << YAML::BeginSeq << v.x << v.y << v.z << YAML::EndSeq;
    return out;
}

YAML::Emitter &operator<<(YAML::Emitter &out, const glm::vec4 &v)
{
    out << YAML::Flow;
    out << YAML::BeginSeq << v.x << v.y << v.z << v.w << YAML::EndSeq;
    return out;
}

YAML::Emitter &operator<<(YAML::Emitter &out, const glm::quat &q)
{
    out << YAML::Flow;
    out << YAML::BeginSeq << q.w << q.x << q.y << q.z << YAML::EndSeq;
    return out;
}


namespace engine
{
    void Serializer::pushComponentDelegate(const SerializeComponentDelegate &delegate)
    {
        mComponentSerializeFunctions.push_back(delegate);
    }
    
    void Serializer::saveComponent(YAML::Emitter &out, Component *component)
    {
        for (const auto &function : mComponentSerializeFunctions)
        {
            if (function(out, component))
                return;
        }
    }
    
    void Serializer::loadComponent(const YAML::Node &node, const Ref<Actor> &actor)
    {
        if (!node["Component"].IsDefined())
            return;
        
        const auto componentType = node["Component"].as<std::string>();
        if (auto it = mLoadComponentFunctions.find(componentType); it != mLoadComponentFunctions.end())
        {
            it->second(node, actor);
            return;
        }
        
        WARN("Could not find a function to load component type: %", componentType);
    }
    
    void Serializer::pushLoadComponent(std::string type, const LoadComponentDelegate &loadComponentDelegate)
    {
        mLoadComponentFunctions.emplace(std::move(type), loadComponentDelegate);
    }
}

namespace engine::serialize
{
    void scene(const std::filesystem::path &path, struct Scene *scene)
    {
        if (path.empty())
            return;
        
        if (!exists(path.parent_path()))
        {
            const bool success = std::filesystem::create_directories(path.parent_path());
            if (!success)
            {
                WARN("Unable to create path for %. Nothing will be saved.", path);
                return;
            }
        }
        
        YAML::Emitter out;
        out << YAML::BeginMap;
        out << YAML::Key << "Scene" << YAML::Value << "Scene Hierarchy";
        out << YAML::Key << "Actors" << YAML::Value << YAML::BeginSeq;
        for (auto &actor : scene->getActors())
            engine::serialize::actor(out, actor.get());
        out << YAML::EndSeq;
        out << YAML::EndMap;
        
        std::ofstream fileOutput(path);
        fileOutput << out.c_str();
        fileOutput.close();
        
        MESSAGE("Scene saved to: %", path);
    }
    
    void actor(YAML::Emitter &out, Actor *actor)
    {
        out << YAML::BeginMap;
        out << YAML::Key << "Actor" << YAML::Value << actor->mName;
        out << YAML::Key << "position" << YAML::Value << actor->position;
        out << YAML::Key << "rotation" << YAML::Value << actor->rotation;
        out << YAML::Key << "scale" << YAML::Value << actor->scale;
        
        out << YAML::Key << "Components" << YAML::Value << YAML::BeginSeq;
        for (auto &component : actor->mComponents)
            serialize::component(out, component.get());
        out << YAML::EndSeq;
        
        out << YAML::Key << "Children" << YAML::Value << YAML::BeginSeq;
        for (auto &child : actor->mChildren)
            serialize::actor(out, child.get());
        out << YAML::EndSeq;
        
        out << YAML::EndMap;
    }
    
    void component(YAML::Emitter &out, Component *component)
    {
        out << YAML::BeginMap;
        serializer->saveComponent(out, component);
        out << YAML::EndMap;
    }
}


