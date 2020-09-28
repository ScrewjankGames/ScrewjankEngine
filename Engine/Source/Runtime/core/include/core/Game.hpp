#pragma once
// STD Headers

// Library Headers

// Engine Headers

#include "system/Memory.hpp"
#include "event_system/EventSystem.hpp"

namespace Screwjank {
    // Forward declare system classes
    class MemorySystem;

    class Game
    {
      public:
        /**
         * Constructor
         */
        Game();

        /**
         * Destructor
         */
        virtual ~Game();

        /**
         * Launches engine subsystems and starts game
         */
        void Start();

      protected:
        /**
         * Main game loop
         */
        void Run();

      private:
        /** Engine's event system */
        UniquePtr<EventSystem> m_EventSystem;

        /** Engine's memory system */
        MemorySystem* m_MemorySystem;

        /** Current frame time */
        float m_DeltaTime;
    };

    // API function externed to allow users to create custom game classes for main
    extern Game* CreateGame();
} // namespace Screwjank
