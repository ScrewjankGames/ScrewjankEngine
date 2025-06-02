#pragma once
// STD Headers

// Library Headers

// Engine Headers
#include <ScrewjankEngine/framework/systems/CameraSystem.hpp>
#include <ScrewjankEngine/framework/systems/InputSystem.hpp>
#include <ScrewjankStd/Log.hpp>

import sj.engine.framework;
import sj.std.memory;
import sj.std.containers;

namespace sj {
    // Forward declarations
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

        /**
         * Loads contents of scene file into game 
         */
        void LoadScene(const char* path);

        /** Returns number of frames fully simulated since game start */
        static uint64_t GetFrameCount(); 

        static float GetDeltaTime();
        
      protected:
        /**
         * Main game loop
         */
        void Run();

        void ShutDown();

      private:
        ECSRegistry m_ecs;
        sj::dynamic_vector<std::unique_ptr<Scene>> m_scenes;

        Window* m_Window = nullptr;
        Renderer* m_Renderer = nullptr;

        InputSystem m_InputSystem;
        CameraSystem m_CameraSystem;

        static uint64_t s_FrameCount;
        static float s_DeltaTime;
    };

    // API function externed to allow users to create custom game classes for main
    extern Game& CreateGame();
} // namespace sj
