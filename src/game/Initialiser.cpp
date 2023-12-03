/**
 * @file Initialiser.cpp
 * @author Ryan Purse
 * @date 03/12/2023
 */


#include "Initialiser.h"

#include "Core.h"
#include "MyScene.h"
#include "EngineState.h"
#include "FileLoader.h"
#include "MyTestActor.h"
#include "Rotator.h"

void initComponentsForEngine()
{
    engine::editor->addComponentOption<Rotator>("Rotator", [](Ref<engine::Actor> actor) {
        actor->addComponent(makeResource<Rotator>());
    });

    engine::editor->addMenuOption("Rotating Cube", []() {
        Ref<engine::Actor> actor = engine::Editor::createDefaultShape("Rotating Cube", (file::modelPath() / "defaultObjects/DefaultCube.glb").string());
        actor->addComponent(makeResource<Rotator>());
        return actor;
    });

    engine::serializer->pushSaveComponent<Rotator>();
    engine::serializer->pushLoadComponent("Rotator", [](const YAML::Node &node, Ref<engine::Actor> actor) {
        const auto rotation = node["Rotation"].as<glm::vec3>();
        auto rotator = actor->addComponent(makeResource<Rotator>(rotation));
    });
    engine::serializer->pushSaveActor<MyTestActor>();
    engine::serializer->pushLoadActor("MyTestActor", [](const YAML::Node &node, engine::Scene *scene) -> Ref<engine::Actor> {
        const float timer = node["Timer"].as<float>();
        const int count = node["Count"].as<int>();
        return scene->spawnActor<MyTestActor>(timer, count);
    });
    engine::serializer->pushSaveScene<MyScene>();
    engine::serializer->pushLoadScene("MyScene", [](const YAML::Node &node) -> std::unique_ptr<engine::Scene> {
        auto myScene = std::make_unique<MyScene>();
        myScene->setLuminanceMultiplier(node["LuminanceMultiplier"].as<float>());
        return myScene;
    });
}
