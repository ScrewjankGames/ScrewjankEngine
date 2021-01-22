#include "core/Game.hpp"

// STD Headers
#include <chrono>

// Library Headers
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

// Engine Headers
#include "core/Window.hpp"
#include "event_system/EventSystem.hpp"
#include "rendering/Renderer.hpp"

namespace sj {

    Game::Game() : m_DeltaTime(0), m_MemorySystem(nullptr), m_EventSystem(nullptr)
    {
    }

    Game::~Game()
    {
        SJ_ENGINE_LOG_INFO("Game terminated");
    }

    void Game::Start()
    {
        m_MemorySystem = MemorySystem::Get();
        m_MemorySystem->Initialize();

        SJ_ENGINE_LOG_INFO("Creating window");
        m_Window = Window::Create();

        m_Renderer = MakeUnique<Renderer>(MemorySystem::GetDefaultAllocator());
        m_EventSystem = MakeUnique<EventSystem>(MemorySystem::GetDefaultAllocator());

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
        }
    }
} // namespace sj
