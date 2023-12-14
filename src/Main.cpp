#include "Core.h"
#include "EngineState.h"
#include "GameInput.h"
#include "game/Initialiser.h"

int main()
{
    engine::Core core({ 1920, 1080 }, true);

    const std::shared_ptr<GameInput> input = std::make_shared<GameInput>();
    gameInput = input.get();
    engine::eventHandler->linkUserEvents(input);

    initComponentsForEngine();
    core.run();
    
    return 0;
}
