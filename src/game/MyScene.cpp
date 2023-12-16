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

void MyScene::onPreRender()
{
}

void MyScene::onDrawUi()
{
    if (engine::ui::Button("Skybox 1"))
        graphics::renderer->generateSkybox((file::texturePath() / "hdr/newport/NewportLoft.hdr").string(), glm::ivec2(512));
    engine::ui::SameLine();
    if (engine::ui::Button("Skybox 2"))
        graphics::renderer->generateSkybox((file::texturePath() / "hdr/norway/Norway.hdr").string(), glm::ivec2(512));
    if (engine::ui::DragFloat("Luminance Multiplier", &mLuminanceMultiplier))
        graphics::renderer->setIblMultiplier(mLuminanceMultiplier);

    static bool yes = true;
    engine::ui::ShowDemoWindow(&yes);
}

void MyScene::setLuminanceMultiplier(const float multiplier)
{
    mLuminanceMultiplier = multiplier;
}

MyScene::~MyScene()
{
}

void serializeScene(engine::serialize::Emitter &out, MyScene *const scene)
{
    out << engine::serialize::Key << "Type" << engine::serialize::Value << "MyScene";
    out << engine::serialize::Key << "LuminanceMultiplier" << scene->mLuminanceMultiplier;
}

