#include "Core.h"
#include "MyScene.h"
#include "EngineState.h"
#include "Rotator.h"

int main()
{
    engine::Core core({ 1920, 1080 }, true);
    engine::serializer->pushSaveComponent<Rotator>();
    engine::serializer->pushLoadComponent("Rotator", [](const YAML::Node &node, Ref<engine::Actor> actor) {
        const auto rotation = node["Rotation"].as<glm::vec3>();
        auto rotator = actor->addComponent(makeResource<Rotator>(rotation));
    });
    
    core.setScene(std::make_unique<MyScene>(), "");
    core.run();
    
    return 0;
}
