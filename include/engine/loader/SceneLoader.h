/**
 * @file SceneLoader.h
 * @author Ryan Purse
 * @date 15/10/2023
 */


#pragma once

#include <filesystem>
#include "Pch.h"
#include "EngineMemory.h"
#include <yaml-cpp/yaml.h>

namespace engine
{
    class Scene;
    class Actor;
}

namespace load
{
    void scene(const std::filesystem::path &path, engine::Scene *scene);
    void actor(const YAML::Node &actorNode, engine::Scene *scene);
    void actor(const YAML::Node &actorNode, Ref<engine::Actor> parent);
}
