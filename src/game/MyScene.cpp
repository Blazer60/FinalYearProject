/**
 * @file MyScene.cpp
 * @author Ryan Purse
 * @date 13/06/2023
 */


#include "MyScene.h"

#include <Engine.h>

MyScene::MyScene()
{
}

void MyScene::onFixedUpdate()
{
}

void MyScene::onUpdate()
{
}

void MyScene::onRender()
{
}

void MyScene::onImguiUpdate()
{
    if (ImGui::Button("Skybox 1"))
        graphics::renderer->generateSkybox((file::texturePath() / "hdr/newport/NewportLoft.hdr").string(), glm::ivec2(512));
    ImGui::SameLine();
    if (ImGui::Button("Skybox 2"))
        graphics::renderer->generateSkybox((file::texturePath() / "hdr/norway/Norway.hdr").string(), glm::ivec2(512));
    if (ImGui::DragFloat("Luminance Multiplier", &mLuminanceMultiplier))
        graphics::renderer->setIblMultiplier(mLuminanceMultiplier);

    static bool yes = true;
    ImGui::ShowDemoWindow(&yes);
}

void MyScene::onImguiMenuUpdate()
{
}

void MyScene::setLuminanceMultiplier(const float multiplier)
{
    mLuminanceMultiplier = multiplier;
}

MyScene::~MyScene()
{
}

void serializeScene(YAML::Emitter &out, MyScene *const scene)
{
    out << YAML::Key << "Type" << YAML::Value << "MyScene";
    out << YAML::Key << "LuminanceMultiplier" << scene->mLuminanceMultiplier;
}

