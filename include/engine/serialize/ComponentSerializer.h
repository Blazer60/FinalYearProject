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
    class DirectionalLight;
    class PointLight;
    
    void attachComponentSerialization();
}
// In global namespace so that uer can override with their own functions.

void serializeComponent(YAML::Emitter &, engine::MeshRenderer *);
void serializeComponent(YAML::Emitter &, engine::DirectionalLight *);
void serializeComponent(YAML::Emitter &, engine::PointLight *);

