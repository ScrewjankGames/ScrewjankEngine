// STD Headers
#include <chrono>

// Library Headers
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

// Engine Headers
#include "core/Game.hpp"
#include "core/Log.hpp"

namespace Screwjank {

    Game::Game() : m_DeltaTime(0)
    {
    }

    Game::~Game()
    {
        SJ_ENGINE_LOG_INFO("Game terminated");
    }

    void Game::Start()
    {
        SJ_ENGINE_LOG_INFO("Initializing core engine systems...");

        SJ_ENGINE_LOG_INFO("Core engine systems initialized.");

        Run();
    }

    void Game::Run()
    {
        using Timer = std::chrono::high_resolution_clock;
        auto previousTime = Timer::now();
        auto currentTime = Timer::now();

        glfwInit();
        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        GLFWwindow* window = glfwCreateWindow(800, 600, "Vulkan window", nullptr, nullptr);

        VkApplicationInfo appInfo = {};
        appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
        appInfo.pNext = nullptr;
        appInfo.pApplicationName = "Screwjank Engine Game";
        appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
        appInfo.pEngineName = "No Engine";
        appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
        appInfo.apiVersion = VK_API_VERSION_1_2;

        VkInstanceCreateInfo createInfo = {};
        createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
        // createInfo.pApplicationInfo = &appInfo;
        createInfo.enabledLayerCount = 0;

        uint32_t glfwExtensionCount = 0;
        const char** glfwExtensions;
        glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);
        createInfo.enabledExtensionCount = glfwExtensionCount;
        createInfo.ppEnabledExtensionNames = glfwExtensions;

        VkInstance vulkanInstance;
        if (vkCreateInstance(&createInfo, nullptr, &vulkanInstance) != VK_SUCCESS) {
            SJ_ENGINE_LOG_ERROR("Failed to create Vulkan instance.");
            return;
        }

        uint32_t extensionCount = 0;
        vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);
        SJ_ENGINE_LOG_INFO("Vulkan loaded with {} extensions supported", extensionCount);

        while (!glfwWindowShouldClose(window)) {
            currentTime = Timer::now();
            m_DeltaTime = std::chrono::duration<float>(currentTime - previousTime).count();

            // Update
            glfwPollEvents();

            previousTime = currentTime;
        }

        vkDestroyInstance(vulkanInstance, nullptr);
        glfwDestroyWindow(window);
        glfwTerminate();
    }
} // namespace Screwjank
