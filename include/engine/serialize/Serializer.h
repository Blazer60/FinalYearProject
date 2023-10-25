/**
 * @file SceneSerializer.h
 * @author Ryan Purse
 * @date 13/10/2023
 */


#pragma once

#include "Pch.h"

#include "ComponentSerializer.h"
#include "EngineMemory.h"

#include <yaml-cpp/yaml.h>
#include <filesystem>
#include <functional>


namespace engine
{
    class Scene;
    class Actor;
    class Component;
}

typedef std::function<bool(YAML::Emitter &out, class engine::Component*)> SerializeComponentDelegate;
typedef std::function<void(const YAML::Node &, Ref<engine::Actor>)> LoadComponentDelegate;

namespace engine
{
    class Serializer
    {
    public:
        Serializer() = default;
        void saveComponent(YAML::Emitter &out, Component *component);
        void loadComponent(const YAML::Node &node, const Ref<Actor> &actor);
        
        template<typename T>
        void pushSaveComponent();
        
        void pushLoadComponent(std::string type, const LoadComponentDelegate &loadComponentDelegate);
        
    protected:
        void pushComponentDelegate(const SerializeComponentDelegate &delegate);
        
        std::vector<SerializeComponentDelegate> mComponentSerializeFunctions;
        std::unordered_map<std::string, LoadComponentDelegate> mLoadComponentFunctions;
    };
}

YAML::Emitter &operator<<(YAML::Emitter &out, const glm::vec2 &v);
YAML::Emitter &operator<<(YAML::Emitter &out, const glm::vec3 &v);
YAML::Emitter &operator<<(YAML::Emitter &out, const glm::vec4 &v);
YAML::Emitter &operator<<(YAML::Emitter &out, const glm::quat &q);

namespace engine::serialize
{
    void scene(const std::filesystem::path &path, Scene* scene);
    void actor(YAML::Emitter &out, Actor *actor);
    void component(YAML::Emitter &out, Component *component);
}

// Used within the engine due to the namespace. We cannot forward declare and friend at the same time in a namespace.
#define ENGINE_SERIALIZABLE_COMPONENT(class) friend void ::serializeComponent(YAML::Emitter&, class*)
#define FORWARD_COMPONENT(cl) class cl; void serializeComponent(YAML::Emitter &, cl *)

// Create a serializeComponent() function and then add it using engine::serializer->pushComponent<T>();
#define SERIALIZABLE_COMPONENT(class) friend void serializeComponent(YAML::Emitter&, class*)

template<typename T>
void engine::Serializer::pushSaveComponent()
{
    pushComponentDelegate([](YAML::Emitter &out, Component *component) -> bool {
        if (auto x = dynamic_cast<T*>(component); x != nullptr)
        {
            serializeComponent(out, x);
            return true;
        }
        
        return false;
    });
}

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
    struct convert<glm::vec2>
    {
        static Node encode(const glm::vec2 &rhs)
        {
            Node node;
            node.push_back(rhs.x);
            node.push_back(rhs.y);
            return node;
        }
        
        static bool decode(const Node& node, glm::vec2 &rhs)
        {
            if (!node.IsSequence() || node.size() != 2)
                return false;
            
            rhs.x = node[0].as<float>();
            rhs.y = node[1].as<float>();
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
