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
        void pushComponentDelegate(const SerializeDelegate &delegate);
        void component(YAML::Emitter &out, Component *component);
        
    protected:
        std::vector<SerializeDelegate> mComponentFunctions;
    };
}

namespace engine::serialize
{
    void scene(const std::filesystem::path &path, Scene* scene);
    void actor(YAML::Emitter &out, Actor *actor);
    void component(YAML::Emitter &out, Component *component);
}

#define SERIALIZABLE(class) friend void ::serializeComponent(YAML::Emitter&, class*)
