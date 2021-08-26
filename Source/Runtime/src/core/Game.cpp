#include <core/Game.hpp>

// STD Headers
#include <chrono>

// Library Headers
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

// Engine Headers
#include <core/Window.hpp>
#include <rendering/Renderer.hpp>

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
        SJ_ENGINE_LOG_INFO("Creating window");
        m_Window = Window::Create();

        m_Renderer = MakeUnique<Renderer>(MemorySystem::GetRootHeapZone(), m_Window.Get());

        Run();
    }

    void Game::Run()
    {
        using Timer = std::chrono::high_resolution_clock;
        auto previousTime = Timer::now();
        auto currentTime = Timer::now();

        while (!m_Window->WindowClosed()) {
            currentTime = Timer::now();
            m_DeltaTime = std::chrono::duration<float>(currentTime - previousTime).count();

            m_Window->ProcessEvents();

            previousTime = currentTime;

            m_FrameCount++;
        }
    }
} // namespace sj
