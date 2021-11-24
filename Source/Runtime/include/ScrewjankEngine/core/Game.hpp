#pragma once
// STD Headers

// Library Headers

// Engine Headers
#include <ScrewjankEngine/core/Log.hpp>
#include <ScrewjankEngine/system/Memory.hpp>

namespace sj {
    // Forward declarations
    class MemorySystem;
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

        /** Returns number of frames fully simulated since game start */
        static uint64_t GetFrameCount(); 

      protected:
        /**
         * Main game loop
         */
        void Run();

      private:
        static uint64_t m_FrameCount;

        /** Handle to game's window */
        Window* m_Window;

        /** Engine's rendering subsystem */
        UniquePtr<Renderer> m_Renderer;

        /** Current frame time */
        float m_DeltaTime;
    };

    // API function externed to allow users to create custom game classes for main
    extern Game* CreateGame();
} // namespace sj
