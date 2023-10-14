/**
 * @file SceneSerializer.h
 * @author Ryan Purse
 * @date 13/10/2023
 */


#pragma once

#include "Pch.h"
#include <yaml-cpp/emitter.h>
#include <filesystem>
#include <functional>


namespace engine
{
    class Scene;
    class Actor;
    class Component;
}

typedef std::function<bool(YAML::Emitter &out, class engine::Component*)> SerializeDelegate;

namespace engine::serialize
{
    const int componentFunctionsCountMax = 64;
    extern SerializeDelegate serializeComponentFunctions[componentFunctionsCountMax];
    extern int componentFunctionsCount;
    
    void pushComponentDelegate(const SerializeDelegate &delegate);
    
    void scene(const std::filesystem::path &path, Scene* scene);
    
    void actor(YAML::Emitter &out, Actor *actor);
    void component(YAML::Emitter &out, Component *component);
}
