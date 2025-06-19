// Screwjank Headers
#include <ScrewjankStd/Log.hpp>

import sj.engine.system.memory.MemorySystem;
import sj.engine.framework.Game;

// Initialize engine global static objects so they are 
// destroyed in the correct order during shutdown
void InitEngineStatics()
{
    sj::MemorySystem::Init();

    #ifndef SJ_GOLD
    SJ_ENGINE_LOG_INFO("Initializing...");
    #endif
}

int main([[maybe_unused]] int argc, [[maybe_unused]] char** argv)
{
    InitEngineStatics();

    sj::Game& game = sj::CreateGame();
    game.Start();
}
