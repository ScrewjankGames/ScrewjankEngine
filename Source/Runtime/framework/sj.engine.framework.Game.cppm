module;

// STD Headers
#include <chrono>

// Library Headers
#include <GLFW/glfw3.h>
#include <imgui.h>

// Engine Headers
#include <ScrewjankEngine/framework/Window.hpp>
#include <ScrewjankStd/Log.hpp>
#include <memory>

export module sj.engine.framework.Game;

import sj.std.memory;
import sj.std.containers.vector;

import sj.engine.ecs.components;

import sj.engine.system.memory.MemorySystem;

import sj.engine.ecs;
import sj.engine.framework.Engine;
import sj.engine.framework.Scene;
import sj.engine.ecs.systems.CameraSystem;
import sj.engine.ecs.systems.InputSystem;

import sj.engine.rendering.Renderer;

export namespace sj
{
    class Game
    {
    public:
        virtual std::string_view GetGameName() = 0;

        /**
         * Constructor
         */
        Game()
            : m_ecs(100, MemorySystem::GetRootMemoryResource()),
              m_scenes(MemorySystem::GetRootMemoryResource())
        {
            auto registerFn = []<class T>(ECSRegistry& registry) {
                registry.RegisterComponentType<T>();
            };

            g_componentTypes.for_each<registerFn>(m_ecs);
        }

        /**
         * Destructor
         */
        virtual ~Game() = default;

        /**
         * Launches engine subsystems and starts game
         */
        void Start()
        {
            Engine::RegisterGameName(GetGameName());

#ifndef SJ_GOLD
            // Setup Dear ImGui context
            IMGUI_CHECKVERSION();
            ImGui::CreateContext();
            ImGuiIO& io = ImGui::GetIO();
            io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard; // Enable Keyboard Controls
            io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad; // Enable Gamepad Controls
            // Setup Dear ImGui style
            ImGui::StyleColorsDark();
#endif
            // Initialize systems
            m_Window = Window::GetInstance();
            m_Window->Init();
            
            {
                MemoryResourceScope _(MemorySystem::GetRootMemoryResource());
                m_Renderer = std::make_unique<Renderer>();
            }

            m_Renderer->Init();
            Engine::RegisterRenderer(m_Renderer.get());
            Engine::RegisterECSRegistry(&m_ecs);

            LoadScene("Data/Engine/Scenes/Default.sj_scene");

            Run();
        }

        /**
         * Loads contents of scene file into game
         */
        void LoadScene(const char* path)
        {
            MemoryResourceScope _(MemorySystem::GetRootMemoryResource());
            m_scenes.emplace_back(new Scene(m_ecs, path));
        }

        /** Returns number of frames fully simulated since game start */
        static uint64_t GetFrameCount()
        {
            return s_FrameCount;
        }

        static float GetDeltaTime()
        {
            return s_DeltaTime;
        }

    protected:
        /**
         * Main game loop
         */
        void Run()
        {
            using Timer = std::chrono::high_resolution_clock;
            auto previousTime = Timer::now();
            auto currentTime = Timer::now();

            while(!m_Window->IsWindowClosed())
            {
                currentTime = Timer::now();
                s_DeltaTime = std::chrono::duration<float>(currentTime - previousTime).count();
                constexpr float kMaxDeltaTime = 1.0f / 15.0f;
                if(s_DeltaTime > kMaxDeltaTime)
                {
                    SJ_ENGINE_LOG_WARN("Large delta time detected- {}. Capping at {}",
                                       s_DeltaTime,
                                       kMaxDeltaTime)
                    s_DeltaTime = kMaxDeltaTime;
                }

                m_Renderer->StartRenderFrame();
                m_Window->ProcessEvents();
                m_InputSystem.Process();
                m_CameraSystem.Process(m_ecs, s_DeltaTime);

                if constexpr(g_IsDebugBuild)
                {
                    if(ImGui::Begin("Status"))
                    {
                        ImGui::Text("timestep: %f", s_DeltaTime);
                    }
                    ImGui::End();
                }

                m_Renderer->Render(m_CameraSystem.GetOutputCameraMatrix());
                s_FrameCount++;
                previousTime = currentTime;
            }

            ShutDown();
        }

        virtual void UpdateGameLogic(float deltaTime) = 0;

        void ShutDown()
        {
            m_Renderer->DeInit();
            m_Window->DeInit();
        }

    private:
        ECSRegistry m_ecs;
        sj::dynamic_vector<std::unique_ptr<Scene>> m_scenes;

        Window* m_Window = nullptr;
        std::unique_ptr<Renderer> m_Renderer = nullptr;

        InputSystem m_InputSystem;
        CameraSystem m_CameraSystem;

        static uint64_t s_FrameCount;
        static float s_DeltaTime;
    };

    // API function externed to allow users to create custom game classes for main
    extern "C++" Game& CreateGame();
} // namespace sj

namespace sj
{
    uint64_t Game::s_FrameCount = 0;
    float Game::s_DeltaTime = 0.0f;
} // namespace sj