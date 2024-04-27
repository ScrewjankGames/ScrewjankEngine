#include <ScrewjankEngine/core/Game.hpp>

// STD Headers
#include <chrono>

// Library Headers
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

// Engine Headers
#include <ScrewjankEngine/core/Window.hpp>
#include <ScrewjankEngine/rendering/Renderer.hpp>
#include <ScrewjankEngine/system/threading/ThreadPool.hpp>

// Dependencies
#include <imgui.h>

namespace sj {

    uint64_t Game::m_FrameCount = 0;

    uint64_t Game::GetFrameCount()
    {
        return m_FrameCount;
    }

    Game::Game() : m_DeltaTime(0)
    {
    }

    Game::~Game()
    {
        SJ_ENGINE_LOG_INFO("Game terminated");
    }

    void Game::Start()
    {
        m_Window = Window::GetInstance();

        // Setup Dear ImGui context
        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImGuiIO& io = ImGui::GetIO();
        (void)io;
        io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard; // Enable Keyboard Controls
        io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad; // Enable Gamepad Controls

        // Setup Dear ImGui style
        ImGui::StyleColorsDark();

        Run();
    }

    void Game::Run()
    {
        using Timer = std::chrono::high_resolution_clock;
        auto previousTime = Timer::now();
        auto currentTime = Timer::now();

        while (!m_Window->IsWindowClosed()) {
            currentTime = Timer::now();
            m_DeltaTime = std::chrono::duration<float>(currentTime - previousTime).count();

            m_Window->ProcessEvents();

            m_InputSystem.Process();

            previousTime = currentTime;

            m_Renderer.Render();
            m_FrameCount++;
        }
    }
} // namespace sj
