#pragma once
// STD Headers

// Library Headers

// Engine Headers
#include "system/Memory.hpp"

namespace Screwjank {

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
        /** Current frame time */
        float m_DeltaTime;

        /** Engine memory sub-system */
        MemorySystem m_MemorySystem;

        /**
         * Main game loop
         */
        void Run();
    };

    // API function externed to allow users to create custom game classes for main
    extern Game* CreateGame();
} // namespace Screwjank
