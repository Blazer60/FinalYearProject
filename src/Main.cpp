#include <Engine.h>

#include "GameInput.h"
#include "game/ExampleScene.h"
#include "game/Initialiser.h"

int main()
{
    engine::Core core({ 1920, 1080 }, true);

    const std::shared_ptr<GameInput> input = std::make_shared<GameInput>();
    gameInput = input.get();
    engine::eventHandler->linkUserEvents(input);

    initComponentsForEngine();

    core.setScene(std::make_unique<ExampleScene>());

    core.run();
    
    return 0;
}
