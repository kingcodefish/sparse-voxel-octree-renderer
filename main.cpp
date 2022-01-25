#include "Engine.h"

int main()
{
    VulkanEngine engine;

    engine.init();
    engine.run();
    engine.cleanup();

    return 0;
}