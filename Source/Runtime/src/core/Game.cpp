#include <ScrewjankEngine/core/Game.hpp>

// STD Headers
#include <chrono>

// Library Headers
#include <GLFW/glfw3.h>

// Engine Headers
#include <ScrewjankEngine/core/Window.hpp>
#include <ScrewjankEngine/rendering/Renderer.hpp>
#include <ScrewjankEngine/system/threading/ThreadPool.hpp>
#include <ScrewjankEngine/system/memory/Memory.hpp>
#include <ScrewjankEngine/system/memory/MemSpace.hpp>

#include <ScrewjankShared/utils/Log.hpp>

// Dependencies
#include <imgui.h>

namespace sj {

    uint64_t Game::s_FrameCount = 0;
    float Game::s_DeltaTime = 0.0f;
    ScriptFactory Game::s_scriptFactory;

    void Game::LoadScene(const char* path)
    {
        MemSpaceScope _ (MemorySystem::GetRootMemSpace());
        m_Scene = std::make_unique<Scene>(path);
    }

    uint64_t Game::GetFrameCount()
    {
        return s_FrameCount;
    }

    float Game::GetDeltaTime()
    {
        return s_DeltaTime;
    }

    Game::Game() 
    {
    }

    Game::~Game()
    {
    }

    void Game::Start()
    {        
        RegisterScriptComponents(s_scriptFactory);

        // Setup Dear ImGui context
        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImGuiIO& io = ImGui::GetIO();
        (void)io;
        io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard; // Enable Keyboard Controls
        io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad; // Enable Gamepad Controls

        // Setup Dear ImGui style
        ImGui::StyleColorsDark();
        // Initialize systems
        m_Window = Window::GetInstance();
        m_Window->Init();

        m_Renderer = Renderer::GetInstance();
        m_Renderer->Init();
        
        LoadScene("Data/Engine/Scenes/Default.sj_scene");

        Run();
    }

    void Game::Run()
    {
        using Timer = std::chrono::high_resolution_clock;
        auto previousTime = Timer::now();
        auto currentTime = Timer::now();

        while (!m_Window->IsWindowClosed()) {
            currentTime = Timer::now();
            s_DeltaTime = std::chrono::duration<float>(currentTime - previousTime).count();
            constexpr float kMaxDeltaTime = 1.0f / 15.0f;
            if(s_DeltaTime > kMaxDeltaTime)
            {
                SJ_ENGINE_LOG_WARN("Large delta time detected- {}. Capping at {}", s_DeltaTime, kMaxDeltaTime)
                s_DeltaTime = kMaxDeltaTime;
            }

            m_Renderer->StartRenderFrame();
            m_Window->ProcessEvents();
            m_InputSystem.Process();
            m_ScriptSystem.Process(m_Scene.get(), s_DeltaTime);
            m_CameraSystem.Process(m_Scene.get(), s_DeltaTime);

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
    
    void Game::ShutDown()
    {
        m_Window->DeInit();
        m_Renderer->DeInit();
    }

} // namespace sj
