// Engine Headers
#include <ScrewjankEngine/framework/Game.hpp>

// Shared Headers
#include <ScrewjankStd/Log.hpp>

import sj.engine.system.memory.MemorySystem;

// Initialize engine global static objects so they are 
// destroyed in the correct order during shutdown
void InitEngineStatics()
{
    sj::MemorySystem::Init();

    #ifndef SJ_GOLD
    SJ_ENGINE_LOG_INFO("Initializing...");
    #endif
}

int main([[maybe_unused]] int arc, [[maybe_unused]] char** argv)
{
    InitEngineStatics();

    sj::Game& game = sj::CreateGame();
    game.Start();
}
