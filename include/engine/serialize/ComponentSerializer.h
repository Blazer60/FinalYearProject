/**
 * @file ComponentSerializer.h
 * @author Ryan Purse
 * @date 15/10/2023
 */


#pragma once

#include "Pch.h"
#include "yaml-cpp/emitter.h"

namespace engine
{
    // Forward declares need to appear here because of the namespace.
    class MeshRenderer;
    struct DirectionalLight;
    struct PointLight;
    struct Spotlight;
    struct DistantLightProbe;
    class SoundComponent;
    class BoxCollider;
    class SphereCollider;
    class RigidBody;
    class MeshCollider;
    class Camera;

    void attachComponentSerialization();
}
// In global namespace so that uer can override with their own functions.

void serializeComponent(YAML::Emitter &, engine::MeshRenderer *);
void serializeComponent(YAML::Emitter &, engine::DirectionalLight *);
void serializeComponent(YAML::Emitter &, engine::PointLight *);
void serializeComponent(YAML::Emitter &, engine::Spotlight *);
void serializeComponent(YAML::Emitter &, engine::DistantLightProbe *);
void serializeComponent(YAML::Emitter &, engine::SoundComponent *);
void serializeComponent(YAML::Emitter &, engine::BoxCollider *);
void serializeComponent(YAML::Emitter &, engine::SphereCollider *);
void serializeComponent(YAML::Emitter &, engine::RigidBody *);
void serializeComponent(YAML::Emitter &, engine::MeshCollider *);
void serializeComponent(YAML::Emitter &, engine::Camera *);
