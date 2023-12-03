#include "Core.h"
#include "game/Initialiser.h"

int main()
{
    engine::Core core({ 1920, 1080 }, true);
    initComponentsForEngine();
    core.run();
    
    return 0;
}
