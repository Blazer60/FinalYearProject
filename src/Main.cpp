#include "Core.h"
#include "MyScene.h"
#include "EngineState.h"
#include "Rotator.h"

int main()
{
    engine::Core core({ 1920, 1080 }, true);
    engine::serializer->pushComponent<Rotator>();
    
    core.setScene(std::make_unique<MyScene>());
    core.run();
    
    return 0;
}
