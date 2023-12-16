/**
 * @file ExampleScene.h
 * @author Ryan Purse
 * @date 16/12/2023
 */


#pragma once

#include "Pch.h"
#include <Engine.h>

void createDefaultMesh(Ref<engine::Actor> actor, const std::filesystem::path &path);

/**
 * @author Ryan Purse
 * @date 16/12/2023
 */
class ExampleScene
    : public engine::Scene
{
public:
    ExampleScene();
    void onImguiUpdate() override;
};
