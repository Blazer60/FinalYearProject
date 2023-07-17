/**
 * @file MyScene.cpp
 * @author Ryan Purse
 * @date 13/06/2023
 */


#include "MyScene.h"
#include "Shader.h"
#include "Vertices.h"
#include "ModelLoader.h"
#include "imgui.h"
#include "gtc/type_ptr.hpp"
#include "TextureLoader.h"
#include "GraphicsState.h"
#include "BloomPass.h"
#include "Drawable.h"

MyScene::MyScene() :
    mMainCamera(glm::vec3(0.f, 3.f, 21.f)),
    mDirectionalLight(glm::normalize(glm::vec3(1.f, 1.f, 1.f)), glm::vec3(0.93f, 0.93f, 0.95f), glm::ivec2(4096), 4),
    mStandardShader(std::make_shared<Shader>(
        "../resources/shaders/geometry/standard/Standard.vert",
        "../resources/shaders/geometry/standard/Standard.frag"))
{
    const auto [mesh0, material0] = load::model<StandardVertex, StandardMaterial>(
        "../resources/models/stoneFloor/MedievalStoneFloor.obj", mStandardShader);
    
    mStoneFloorMesh = mesh0;
    for (auto &material : material0)
    {
        material->setHeightMap(load::texture("../resources/models/stoneFloor/TexturesCom_TileStones_1K_height.png"));
        material->setRoughnessMap(load::texture("../resources/models/stoneFloor/TexturesCom_TileStones_1K_roughness.png"));
        material->metallic = 0.f;
        material->roughness = 0.5f;
    }
    mStoneFloorMaterial = toSharedMaterials(material0);
    
    const auto [mesh1, material1] = load::model<StandardVertex>("../resources/models/blueSphere/BlueSphere.obj");
    
    mBall = mesh1[0];
    
    for (int i = 0; i < 10; ++i)
    {
        auto material = std::make_shared<StandardMaterial>();
        material->attachShader(mStandardShader);
        material->ambientColour = glm::vec3(1.f);
        material->roughness = static_cast<float>(i) / 10.f;
        material->metallic = 0.f;
        
        mMaterials.push_back(material);
    }
    
    for (int i = 0; i < 10; ++i)
    {
        auto material = std::make_shared<StandardMaterial>();
        material->attachShader(mStandardShader);
        material->ambientColour = glm::vec3(1.f);
        material->roughness = static_cast<float>(i) / 10.f;
        material->metallic = 1.f;
        
        mMaterials.push_back(material);
    }
}

void MyScene::onFixedUpdate()
{
}

void MyScene::onUpdate()
{
    mMainCamera.update();
    mDirectionalLight.updateLayerCount(graphics::renderer->shadowCascadeZones);
}

void MyScene::onRender()
{
    graphics::renderer->submit(mMainCamera.toSettings());
    graphics::renderer->submit(mDirectionalLight);
    graphics::renderer->drawMesh(mStoneFloorMesh, mStoneFloorMaterial, glm::mat4(1.f));
    
    for (int i = 0; i < 10; ++i)
        graphics::renderer->drawMesh(*mBall, *(mMaterials[i]), glm::translate(glm::mat4(1.f), glm::vec3(-10.f + (i * 2.2f), 1.f, 0.f)));
    for (int i = 0; i < 10; ++i)
        graphics::renderer->drawMesh(*mBall, *(mMaterials[i + 10]), glm::translate(glm::mat4(1.f), glm::vec3(-10.f + (i * 2.2f), 3.2f, 0.f)));
}

void MyScene::onImguiUpdate()
{
    ui::draw(mMainCamera);
    
    ImGui::ColorPicker3("Intensity", glm::value_ptr(mDirectionalLight.intensity), ImGuiColorEditFlags_HDR | ImGuiColorEditFlags_Float);
}

void MyScene::onImguiMenuUpdate()
{
}

MyScene::~MyScene()
{
}

