#ifndef SJ_GOLD

    // STD Headers

// Library Headers

// Screwjank Headers
#include <ScrewjankEngine/utils/Log.hpp>

namespace sj {
    Logger::Logger(const char* name) : m_Name(name)
    {
        ;
    }

    Logger* Logger::GetEngineLogger()
    {
        static Logger s_EngineLogger("Engine");
        return &s_EngineLogger;
    }

    Logger* Logger::GetGameLogger()
    {
        static Logger s_GameLogger("Game");
        return &s_GameLogger;
    }
} // namespace sj

#endif