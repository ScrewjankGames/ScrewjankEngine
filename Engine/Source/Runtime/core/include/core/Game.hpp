#pragma once
// STD Headers

// Library Headers

// Engine Headers
#include "system/Memory.hpp"

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
        /** Engine's memory system */
        MemorySystem* m_MemorySystem;

        /** Current frame time */
        float m_DeltaTime;

        /**
         * Main game loop
         */
        void Run();
    };

    // API function externed to allow users to create custom game classes for main
    extern Game* CreateGame();
} // namespace Screwjank
