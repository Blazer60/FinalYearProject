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

#include "FileLoader.h"

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

    void Serializer::pushActorDelegate(const SerializeActorDelegate& delegate)
    {
        mActorSerializeFunctions.push_back(delegate);
    }

    void Serializer::pushSceneDelegate(const SerializeSceneDelegate& delegate)
    {
        mSceneSerializeFunctions.push_back(delegate);
    }

    void Serializer::saveComponent(YAML::Emitter &out, Component *component) const
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

    void Serializer::saveActor(YAML::Emitter& out, Actor* actor) const
    {
        for (const auto &function : mActorSerializeFunctions)
        {
            if (function(out, actor))
                    return;
        }
    }

    Ref<Actor> Serializer::loadActor(const YAML::Node& node, Scene *scene)
    {
        if (!node["Type"].IsDefined())
            return scene->spawnActor<Actor>();

        const auto actorType = node["Type"].as<std::string>();
        if (auto it = mLoadActorFunctions.find(actorType); it != mLoadActorFunctions.end())
            return it->second(node, scene);

        WARN("Could not find a fuction to load actor of type: %", actorType);
        return scene->spawnActor<Actor>();
    }

    void Serializer::saveScene(YAML::Emitter& out, Scene* scene) const
    {
        for (const auto &function : mSceneSerializeFunctions)
        {
            if (function(out, scene))
                return;
        }
    }

    std::unique_ptr<Scene> Serializer::loadScene(const YAML::Node& node)
    {
        if (!node["Type"].IsDefined())
            return std::make_unique<Scene>();

        const auto sceneType = node["Type"].as<std::string>();
        if (auto it = mLoadSceneFunctions.find(sceneType); it != mLoadSceneFunctions.end())
            return it->second(node);

        WARN("Could not find a function to load scene of type: %", sceneType);
        return std::make_unique<Scene>();
    }

    void Serializer::pushLoadScene(std::string type, const LoadSceneDelegate& loadSceneDelegate)
    {
        mLoadSceneFunctions.emplace(std::move(type), loadSceneDelegate);
    }

    void Serializer::pushLoadActor(std::string type, const LoadActorDelegate& loadActorDelegate)
    {
        mLoadActorFunctions.emplace(std::move(type), loadActorDelegate);
    }

    void Serializer::pushLoadComponent(std::string type, const LoadComponentDelegate &loadComponentDelegate)
    {
        mLoadComponentFunctions.emplace(std::move(type), loadComponentDelegate);
    }
}

namespace engine::serialize
{
    void scene(const std::filesystem::path &path, Scene *scene)
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
        out << YAML::Key << "Scene" << YAML::Value << file::makeRelativeToResourcePath(path).string();
        serializer->saveScene(out, scene);
        out << YAML::Key << "Actors" << YAML::Value << YAML::BeginSeq;
        for (auto &actor : scene->getActors())
            serialize::actor(out, actor.get());
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
        serializer->saveActor(out, actor);
        out << YAML::Key << "Name" << YAML::Value << actor->mName;
        out << YAML::Key << "UUID" << YAML::Value << actor->mId;
        out << YAML::Key << "position" << YAML::Value << actor->position;
        out << YAML::Key << "rotation" << YAML::Value << actor->rotation;
        out << YAML::Key << "scale" << YAML::Value << actor->scale;

        out << YAML::Key << "Components" << YAML::Value << YAML::BeginSeq;
        for (auto &component : actor->mComponents)
            serialize::component(out, component.get());
        out << YAML::EndSeq;
        out << YAML::Key << "Children" << YAML::Value << actor->mChildren;
        out << YAML::EndMap;
    }
    
    void component(YAML::Emitter &out, Component *component)
    {
        out << YAML::BeginMap;
        serializer->saveComponent(out, component);
        out << YAML::EndMap;
    }
}


