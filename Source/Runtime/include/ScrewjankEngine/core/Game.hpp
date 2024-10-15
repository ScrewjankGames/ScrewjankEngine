#pragma once
// STD Headers

// Library Headers

// Engine Headers
#include <ScrewjankEngine/core/systems/CameraSystem.hpp>
#include <ScrewjankEngine/core/systems/InputSystem.hpp>
#include <ScrewjankEngine/core/Scene.hpp>
#include <ScrewjankEngine/utils/Log.hpp>
#include <ScrewjankEngine/system/memory/Memory.hpp>
#include <ScrewjankEngine/core/ScriptFactory.hpp>

namespace sj {
    // Forward declarations
    class MemorySystem;
    class Window;
    class Renderer;

    class Game
    {
      public:

        /**
         * Runtime Configuration functions- fill these out in your game class
         */
         virtual void RegisterScriptComponents(ScriptFactory& factory) = 0;

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

        const ScriptFactory& GetScriptFactory() { return s_scriptFactory; }

      protected:
        /**
         * Main game loop
         */
        void Run();

        void ShutDown();

      private:
        static ScriptFactory s_scriptFactory;

        std::unique_ptr<Scene> m_Scene;

        Window* m_Window;
        Renderer* m_Renderer;

        InputSystem m_InputSystem;
        CameraSystem m_CameraSystem;


        static uint64_t s_FrameCount;
        static float s_DeltaTime;
    };

    // API function externed to allow users to create custom game classes for main
    extern Game* CreateGame();
} // namespace sj
