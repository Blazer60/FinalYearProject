/**
 * @file ExampleScene.cpp
 * @author Ryan Purse
 * @date 16/12/2023
 */


#include "ExampleScene.h"

#include "Lava.h"

void createDefaultMesh(Ref<engine::Actor> actor, const std::filesystem::path& path)
{
    const std::shared_ptr<Shader> shader = load::shader(
        file::shaderPath() / "geometry/standard/Standard.vert",
        file::shaderPath() / "geometry/standard/Standard.frag");

    // Spawning an actor with a custom mesh.
    Ref<engine::MeshRenderer> meshRenderer = actor->addComponent(load::meshRenderer<StandardVertex>(path));
    const auto material = std::make_shared<engine::StandardMaterialSubComponent>();
    material->attachShader(shader);
    material->setAmbientColour(engine::vec3(1.f));
    meshRenderer->addMaterial(material);
}

ExampleScene::ExampleScene()
{
    // Spawning an actor with a custom mesh.
    Ref<engine::Actor> meshActor = spawnActor<engine::Actor>("Mesh Renderer Example");
    Ref<engine::MeshRenderer> meshRenderer = meshActor->addComponent(load::meshRenderer<StandardVertex>(file::modelPath() / "utahTeapot/UtahTeapot.glb"));
    const auto material = std::make_shared<engine::StandardMaterialSubComponent>();
    const std::shared_ptr<Shader> shader = load::shader(
        file::shaderPath() / "geometry/standard/Standard.vert",
        file::shaderPath() / "geometry/standard/Standard.frag");
    material->attachShader(shader);
    material->setAmbientColour(engine::vec3(0.f, 0.3f, 0.f));
    material->setRoughness(0.5f);
    meshRenderer->addMaterial(material);
    meshActor->position = engine::vec3(-7.f, 1.f, -30.f);

    // Creating physics objects.
    Ref<engine::Actor> physicsExample = spawnActor<engine::Actor>("Physics Example");
    physicsExample->position = engine::vec3(0.f, 0.f, -30.f);

    Ref<engine::Actor> sphere = physicsExample->addChildActor(makeResource<engine::Actor>("Sphere"));
    sphere->addComponent(makeResource<engine::SphereCollider>(1.f));
    sphere->addComponent(makeResource<engine::RigidBody>());
    createDefaultMesh(sphere, file::modelPath() / "defaultObjects/DefaultSphere.glb");
    sphere->position = engine::vec3(0.f, 5.f, 0.f);

    Ref<engine::Actor> ground = spawnActor<engine::Actor>("Ground");
    physicsExample->addChildActor(ground);  // Alternative method.
    ground->addComponent(makeResource<engine::BoxCollider>(engine::vec3(10.f, 1.f, 10.f)));
    ground->addComponent(makeResource<engine::RigidBody>(0.f));
    createDefaultMesh(ground, file::modelPath() / "defaultObjects/DefaultCube.glb");
    ground->scale = engine::vec3(10.f, 1.f, 10.f);

    // Creating an audio object.
    Ref<engine::Actor> audioActor = spawnActor<engine::Actor>("Sound Example");
    audioActor->addComponent(makeResource<engine::SoundComponent>(file::resourcePath() / "audio/dixie_horn.ogg"));

    // Custom material.
    Ref<engine::Actor> materialActor = spawnActor<engine::Actor>("Custom Material Example");
    Ref<engine::MeshRenderer> materialMesh = materialActor->addComponent(load::meshRenderer<StandardVertex>(file::modelPath() / "MaterialBall.glb"));
    const auto customMaterial = std::make_shared<Lava>();
    materialMesh->addMaterial(customMaterial);
    materialActor->position = engine::vec3(7.f, 1.f, -30.f);
}

void ExampleScene::onDrawUi()
{
    engine::ui::TextWrapped(
        "Note: Due to the components be created in the scene's constructor,"
        " they will be duplicated if the scene is saved. It is recommend to"
        " use the editor to create new objects.");
}
