/**
 * @file SceneSerializer.cpp
 * @author Ryan Purse
 * @date 13/10/2023
 */


#include "SceneSerializer.h"
#include "Scene.h"
#include "Actor.h"
#include "Component.h"
#include "MeshComponent.h"

#include <yaml-cpp/yaml.h>
#include <fstream>

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


namespace engine::serialize
{
    // static const int componentFunctionsCountMax = 64;
    SerializeDelegate serializeComponentFunctions[componentFunctionsCountMax];
    int componentFunctionsCount = 0;
    
    void pushComponentDelegate(const SerializeDelegate &delegate)
    {
        serializeComponentFunctions[componentFunctionsCount] = delegate;
        componentFunctionsCount++;
    }
    
    void scene(const std::filesystem::path &path, struct Scene *scene)
    {
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
        const int count = componentFunctionsCount;
        for (int i = 0; i < componentFunctionsCount; ++i)
        {
            if (serializeComponentFunctions[i](out, component))
                break;  // One of the functions successfully serialized the component.
        }
        
        // out << YAML::BeginMap;
        // out << YAML::Key << "Component Type" << YAML::Value << "MeshComponent";
        // out << YAML::Key << "show" << YAML::Value << meshComponent->mShow;
        // out << YAML::EndMap;
    }
}

