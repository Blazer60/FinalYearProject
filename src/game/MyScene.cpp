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
#include "MeshComponent.h"

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
    
    std::unique_ptr<engine::Actor> stoneFloor = std::make_unique<engine::Actor>("Stone Floor");
    stoneFloor->addComponent(std::make_unique<engine::MeshComponent>(mesh0, toSharedMaterials(material0)));
    mActors.push_back(std::move(stoneFloor));
    
    const auto [mesh1, material1] = load::model<StandardVertex>("../resources/models/blueSphere/BlueSphere.obj");
    
    mBall = mesh1[0];
    SharedMesh ballMesh = { mesh1[0] };
    
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
    
    for (int i = 0; i < 10; ++i)
    {
        std::unique_ptr<engine::Actor> ball = std::make_unique<engine::Actor>("Ball " + std::to_string(i));
        ball->position = glm::vec3(-10.f + (i * 2.2f), 1.f, 0.f);
        SharedMaterials sharedMaterials = { mMaterials[i] };
        ball->addComponent(std::make_unique<engine::MeshComponent>(ballMesh, sharedMaterials));
        mActors.push_back(std::move(ball));
    }
    for (int i = 0; i < 10; ++i)
    {
        std::unique_ptr<engine::Actor> ball = std::make_unique<engine::Actor>("Ball " + std::to_string(i + 10));
        ball->position = glm::vec3(-10.f + (i * 2.2f), 3.2f, 0.f);
        SharedMaterials sharedMaterials = { mMaterials[i + 10] };
        ball->addComponent(std::make_unique<engine::MeshComponent>(ballMesh, sharedMaterials));
        mActors.push_back(std::move(ball));
    }
    
    std::unique_ptr<engine::Actor> directionalLight = std::make_unique<engine::Actor>("Directional Light");
    directionalLight->addComponent(std::make_unique<DirectionalLight>(glm::normalize(glm::vec3(1.f, 1.f, 1.f)), glm::vec3(0.93f, 0.93f, 0.95f), glm::ivec2(4096), 4));
    mActors.push_back(std::move(directionalLight));
}

void MyScene::onFixedUpdate()
{
}

void MyScene::onUpdate()
{
    mMainCamera.update();
}

void MyScene::onRender()
{
    graphics::renderer->submit(mMainCamera.toSettings());
}

void MyScene::onImguiUpdate()
{
    ui::draw(mMainCamera);
    
    if (ImGui::Button("Skybox 1"))
        graphics::renderer->generateSkybox("../resources/textures/hdr/newport/NewportLoft.hdr", glm::ivec2(512));
    ImGui::SameLine();
    if (ImGui::Button("Skybox 2"))
        graphics::renderer->generateSkybox("../resources/textures/hdr/norway/Norway.hdr", glm::ivec2(512));
    
    // static bool yes = true;
    // ImGui::ShowDemoWindow(&yes);
}

void MyScene::onImguiMenuUpdate()
{
}

MyScene::~MyScene()
{
}

