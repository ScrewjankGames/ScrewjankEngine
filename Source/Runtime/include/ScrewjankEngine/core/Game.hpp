#pragma once
// STD Headers

// Library Headers

// Engine Headers
#include <ScrewjankEngine/core/systems/InputSystem.hpp>
#include <ScrewjankEngine/utils/Log.hpp>
#include <ScrewjankEngine/system/memory/Memory.hpp>

namespace sj {
    // Forward declarations
    class MemorySystem;
    class Window;
    class Renderer;
    class InputSystem;

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

        void ShutDown();

      private:
        static uint64_t m_FrameCount;

        Window* m_Window;
        Renderer* m_Renderer;

        InputSystem m_InputSystem;

        /** Current frame time */
        float m_DeltaTime;
    };

    // API function externed to allow users to create custom game classes for main
    extern Game* CreateGame();
} // namespace sj
