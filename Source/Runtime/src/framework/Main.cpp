// Engine Headers
#include <ScrewjankEngine/framework/Game.hpp>

// Shared Headers
#include <ScrewjankShared/utils/Log.hpp>

import sj.engine.system.memory;

// Initialize engine global static objects so they are 
// destroyed in the correct order during shutdown
void InitEngineStatics()
{
    #ifndef SJ_GOLD
    SJ_ENGINE_LOG_INFO("Initializing...");
    #endif

    sj::MemorySystem::Init();
}

int main(int arc, char** argv)
{
    InitEngineStatics();

    sj::Game& game = sj::CreateGame();
    game.Start();
}
