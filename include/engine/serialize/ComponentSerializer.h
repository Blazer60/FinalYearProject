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
    class MeshRenderer;
    
    void attachComponentSerialization();
}

void serializeComponent(YAML::Emitter &, engine::MeshRenderer *);

