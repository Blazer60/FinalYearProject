#include "Core.h"
#include "MyScene.h"

int main()
{
    engine::Core core({ 1920, 1080 }, true);
    core.setScene(std::make_unique<MyScene>());
    core.run();
    
    return 0;
}
