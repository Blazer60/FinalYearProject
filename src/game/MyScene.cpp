/**
 * @file MyScene.cpp
 * @author Ryan Purse
 * @date 13/06/2023
 */


#include "MyScene.h"
#include "Shader.h"
#include "Vertices.h"
#include "imgui.h"
#include "TextureLoader.h"
#include "GraphicsState.h"
#include "BloomPass.h"
#include "MeshComponent.h"
#include "AssimpLoader.h"
#include "Editor.h"
#include "Rotator.h"
#include "EngineState.h"
#include "SceneSerializer.h"
#include <FileLoader.h>

MyScene::MyScene() :
    mStandardShader(std::make_shared<Shader>(
        file::shaderPath() / "geometry/standard/Standard.vert",
        file::shaderPath() / "geometry/standard/Standard.frag"))
{
    engine::editor->addComponentOption<Rotator>("Rotator", [](Ref<engine::Actor> actor) {
        actor->addComponent(makeResource<Rotator>());
    });
    
    engine::editor->addMenuOption("Rotating Cube", []() {
        Ref<engine::Actor> actor = engine::Editor::createDefaultShape("Rotating Cube", (file::modelPath() / "defaultObjects/DefaultCube.glb").string());
        actor->addComponent(makeResource<Rotator>());
        return actor;
    });
    
    auto floor = spawnActor<engine::Actor>("Floor");
    auto floorMesh = load::model<StandardVertex>(file::modelPath() /  "stoneFloor/MedievalStoneFloor.obj");
    auto floorMaterial = std::make_shared<StandardMaterial>();
    floorMaterial->attachShader(mStandardShader);
    floorMaterial->setDiffuseMap(load::texture(file::modelPath() /  "stoneFloor/TexturesCom_TileStones_1K_albedo.png"));
    floorMaterial->setNormalMap(load::texture(file::modelPath() /  "stoneFloor/TexturesCom_TileStones_1K_normal.png"));
    floorMaterial->setHeightMap(load::texture(file::modelPath() /  "stoneFloor/TexturesCom_TileStones_1K_height.png"));
    floorMaterial->setRoughnessMap(load::texture(file::modelPath() /  "stoneFloor/TexturesCom_TileStones_1K_roughness.png"));
    floorMaterial->metallic = 0.f;
    floorMaterial->roughness = 0.5f;
    floorMaterial->ambientColour = glm::vec3(0.8f);
    floor->addComponent(makeResource<engine::MeshComponent>(floorMesh, floorMaterial));

    auto ballMesh = load::model<StandardVertex>(file::modelPath() / "blueSphere/BlueSphere.obj");

    std::vector<glm::vec3> positions;
    std::vector<float> roughness;
    std::vector<float> metallic;
    for (int i = 0; i < 10; ++i)
    {
        positions.emplace_back(-10.f + (i * 2.2f), 1.f, 0.f);
        roughness.emplace_back(i / 10.f);
        metallic.emplace_back(0.f);
    }
    for (int i = 0; i < 10; ++i)
    {
        positions.emplace_back(-10.f + (i * 2.2f), 3.2f, 0.f);
        roughness.emplace_back(i / 10.f);
        metallic.emplace_back(1.f);
    }

    for (int i = 0; i < 20; ++i)
    {
        auto ball = spawnActor<engine::Actor>(std::string("Ball " + std::to_string(i)));
        ball->position = positions[i];
        auto ballMaterial = std::make_shared<StandardMaterial>();
        ballMaterial->attachShader(mStandardShader);
        ballMaterial->ambientColour = glm::vec3(1.f);
        ballMaterial->roughness = roughness[i];
        ballMaterial->metallic = metallic[i];
        ball->addComponent(makeResource<engine::MeshComponent>(ballMesh, ballMaterial));
    }

    auto directionalLight = spawnActor<engine::Actor>("Directional Light");
    directionalLight->addComponent(makeResource<DirectionalLight>(
        glm::normalize(glm::vec3(1.f, 1.f, 1.f)), glm::vec3(0.93f, 0.93f, 0.95f), glm::ivec2(4096), 4));

    auto teapot = spawnActor<engine::Actor>("Teapot");
    teapot->position = glm::vec3(0.f, 0.f, 5.f);
    teapot->scale = glm::vec3(0.3f);
    SharedMesh teapotMesh = load::model<StandardVertex>(file::modelPath() / "utahTeapot/UtahTeapot.obj");
    auto teapotMaterial = std::make_shared<StandardMaterial>();
    teapotMaterial->attachShader(mStandardShader);
    teapotMaterial->ambientColour = glm::vec3(0.f, 0.4f, 0.01f);
    teapotMaterial->roughness = 0.6f;
    teapotMaterial->metallic = 1.f;
    teapot->addComponent(makeResource<engine::MeshComponent>(teapotMesh, teapotMaterial));
    
    SharedMesh childMesh = load::model<StandardVertex>(file::modelPath() / "defaultObjects/DefaultTorus.glb");
    auto childMaterial = std::make_shared<StandardMaterial>();
    childMaterial->attachShader(mStandardShader);
    childMaterial->ambientColour = glm::vec3(1.f, 0.f, 0.f);
    
    auto parent = spawnActor<engine::Actor>("Parent");
    parent->position = glm::vec3(5.f, 5.f, -12.f);
    auto child = parent->addChildActor(makeResource<engine::Actor>("Child"));
    child->addComponent(makeResource<engine::MeshComponent>(childMesh, childMaterial));
    
    auto pointLight = spawnActor<engine::Actor>("Point Light");
    pointLight->position = glm::vec3(0.f, 6.f, 5.f);
    pointLight->addComponent(makeResource<PointLight>());
    
    auto leatherBall = spawnActor<engine::Actor>("Leather Ball");
    leatherBall->position = glm::vec3(0.f, 5.f, 0.f);
    auto leatherBallModel = load::model<StandardVertex>(file::modelPath() / "leatherBall/LeatherBall.obj");
    auto leatherMaterial = std::make_shared<StandardMaterial>();
    leatherMaterial->attachShader(mStandardShader);
    leatherMaterial->setDiffuseMap(load::texture(file::modelPath() / "leatherBall/TexturesCom_Leather_Tufted_New_1K_albedo.png"));
    leatherMaterial->setNormalMap(load::texture(file::modelPath() / "leatherBall/TexturesCom_Leather_Tufted_New_1K_normal.png"));
    leatherMaterial->setHeightMap(load::texture(file::modelPath() / "leatherBall/TexturesCom_Leather_Tufted_New_1K_height.png"));
    leatherMaterial->setRoughnessMap(load::texture(file::modelPath() / "leatherBall/TexturesCom_Leather_Tufted_New_1K_roughness.png"));
    leatherBall->addComponent(makeResource<engine::MeshComponent>(leatherBallModel, leatherMaterial));
    
    engine::serialize::scene(file::resourcePath() / "scenes/test.pcy", static_cast<engine::Scene*>(this));
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
    
    // static bool yes = true;
    // ImGui::ShowDemoWindow(&yes);
}

void MyScene::onImguiMenuUpdate()
{
}

MyScene::~MyScene()
{
}

