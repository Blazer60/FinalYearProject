/**
 * @file Engine.h
 * @author Ryan Purse
 * @date 16/12/2023
 */


#pragma once

#include "Pch.h"

#ifdef ENGINE_BUILD
    #error "Engine Global Header is not allowed within the engine. Include objects directly".
#endif

#include "Actor.h"
#include "Component.h"
#include "Core.h"
#include "Drawable.h"
#include "EngineMath.h"
#include "EngineMemory.h"
#include "EngineState.h"
#include "EventHandler.h"
#include "Input.h"
#include "Scene.h"
#include "Serializer.h"
#include "SoundComponent.h"
#include "Ui.h"
#include "MeshRenderer.h"
#include "RigidBody.h"
#include "Colliders.h"
#include "ShaderLoader.h"
#include "TextureLoader.h"

#include <GraphicsState.h>
#include <Renderer.h>

namespace engine
{
    using namespace glm;
    namespace ui
    {
        using namespace ImGui;
        using namespace ImGuizmo;
    }
    namespace serialize
    {
        using namespace YAML;
    }
}