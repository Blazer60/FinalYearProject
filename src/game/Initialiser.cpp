/**
 * @file Initialiser.cpp
 * @author Ryan Purse
 * @date 03/12/2023
 */


#include "Initialiser.h"

#include <Engine.h>

#include "CameraController.h"
#include "CollisionInfo.h"
#include "EngineState.h"
#include "LookAtActor.h"
#include "MyScene.h"
#include "MyTestActor.h"
#include "Rotator.h"

void initComponentsForEngine()
{
    engine::editor->addComponentOption<Rotator>("Rotator", [](Ref<engine::Actor> actor) {
        actor->addComponent(makeResource<Rotator>());
    });

    engine::editor->addComponentOption<CollisionInfo>("Collision Info", [](Ref<engine::Actor> actor) {
        actor->addComponent(makeResource<CollisionInfo>());
    });

    engine::editor->addComponentOption<CameraController>("Camera Controller", [](Ref<engine::Actor> actor) {
        actor->addComponent(makeResource<CameraController>());
    });

    engine::editor->addComponentOption<LookAtActor>("Look At Actor", [](Ref<engine::Actor> actor) {
        actor->addComponent(makeResource<LookAtActor>());
    });

    engine::editor->addMenuOption("Rotating Cube", [] {
        Ref<engine::Actor> actor = engine::editor->createDefaultShape("Rotating Cube", (file::modelPath() / "defaultObjects/DefaultCube.glb").string());
        actor->addComponent(makeResource<Rotator>());
        return actor;
    });

    engine::serializer->pushSaveComponent<Rotator>();
    engine::serializer->pushLoadComponent("Rotator", [](const engine::serialize::Node &node, Ref<engine::Actor> actor) {
        const auto rotation = node["Rotation"].as<engine::vec3>();
        auto rotator = actor->addComponent(makeResource<Rotator>(rotation));
    });
    engine::serializer->pushSaveActor<MyTestActor>();
    engine::serializer->pushLoadActor("MyTestActor", [](const engine::serialize::Node &node, engine::Scene *scene) -> Ref<engine::Actor> {
        const float timer = node["Timer"].as<float>();
        const int count = node["Count"].as<int>();
        return scene->spawnActor<MyTestActor>(timer, count);
    });
    engine::serializer->pushSaveScene<MyScene>();
    engine::serializer->pushLoadScene("MyScene", [](const engine::serialize::Node &node) -> std::unique_ptr<engine::Scene> {
        auto myScene = std::make_unique<MyScene>();
        myScene->setLuminanceMultiplier(node["LuminanceMultiplier"].as<float>());
        return myScene;
    });

    engine::serializer->pushSaveComponent<CollisionInfo>();
    engine::serializer->pushLoadComponent("CollisionInfo", [](const engine::serialize::Node &node, Ref<engine::Actor> actor) {
        actor->addComponent(makeResource<CollisionInfo>());
    });

    engine::serializer->pushSaveComponent<CameraController>();
    engine::serializer->pushLoadComponent("CameraController", [](const engine::serialize::Node &node, Ref<engine::Actor> actor) {
        auto cc = actor->addComponent(makeResource<CameraController>());
        if (node["Speed"].IsDefined())
            cc->mSpeed = node["Speed"].as<float>();
        if (node["JumpForce"].IsDefined())
            cc->mJumpForce = node["JumpForce"].as<float>();
    });

    engine::serializer->pushSaveComponent<LookAtActor>();
    engine::serializer->pushLoadComponent("LookAtActor", [](const engine::serialize::Node &node, Ref<engine::Actor> actor) {
        Ref<LookAtActor> component = actor->addComponent(makeResource<LookAtActor>());
        if (node["ActorId"].IsDefined())
        {
            component->trackedActorId = node["ActorId"].as<engine::UUID>();
        }
        if (node["InvertZ"].IsDefined())
        {
            component->invertZ = node["InvertZ"].as<bool>();
        }
    });
}
