#pragma once
// STD Headers

// Library Headers

// Engine Headers
#include "core/Log.hpp"
#include "system/Memory.hpp"

namespace sj {
    // Forward declarations
    class MemorySystem;
    class EventSystem;
    class Window;
    class Renderer;

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
        /** Engine's memory system */
        MemorySystem* m_MemorySystem;

        /** Handle to game's window */
        UniquePtr<Window> m_Window;

        /** Engine's rendering subsystem */
        UniquePtr<Renderer> m_Renderer;

        /** Engine's event system */
        UniquePtr<EventSystem> m_EventSystem;

        /** Current frame time */
        float m_DeltaTime;
    };

    // API function externed to allow users to create custom game classes for main
    extern Game* CreateGame();
} // namespace sj
