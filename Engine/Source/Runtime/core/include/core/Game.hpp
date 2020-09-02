#pragma once
// STD Headers

// Library Headers

// Engine Headers

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
        /**
         * Main game loop
         */
        void Run();

        /** Current frame time */
        float m_DeltaTime;
    };

    // API function externed to allow users to create custom game classes for main
    extern Game* CreateGame();
} // namespace Screwjank
