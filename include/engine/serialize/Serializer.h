/**
 * @file SceneSerializer.h
 * @author Ryan Purse
 * @date 13/10/2023
 */


#pragma once

#include "Pch.h"

#include "ComponentSerializer.h"

#include <yaml-cpp/yaml.h>
#include <filesystem>
#include <functional>


namespace engine
{
    class Scene;
    class Actor;
    class Component;
}

typedef std::function<bool(YAML::Emitter &out, class engine::Component*)> SerializeDelegate;

namespace engine
{
    class Serializer
    {
    public:
        Serializer() = default;
        void component(YAML::Emitter &out, Component *component);
        
        template<typename T>
        void pushComponent();
        
    protected:
        void pushComponentDelegate(const SerializeDelegate &delegate);
        
        std::vector<SerializeDelegate> mComponentFunctions;
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
void engine::Serializer::pushComponent()
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
