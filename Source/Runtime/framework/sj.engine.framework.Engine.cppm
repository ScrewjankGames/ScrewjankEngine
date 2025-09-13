module;

#include <ScrewjankStd/Assert.hpp>
#include <ScrewjankStd/Log.hpp>

#include <memory>
#include <concepts>

#ifndef SJ_VERSION
    #include <imgui.h>
#endif

export module sj.engine.framework.Engine;
import sj.engine.rendering.Renderer;
import sj.engine.system.threading.ThreadContext;
import sj.engine.system.memory.MemorySystem;
import sj.engine.system.Timer;
import sj.engine.framework.Window;
import sj.engine.ecs;
import sj.engine.ecs.components;
import sj.std.memory.literals;
import sj.std.math;

export namespace sj
{
    /**
     * Service locator to provide global access to engine systems
     */
    class Engine
    {
    public:
        Engine(uint64_t rootHeapSize)
        {
            sj::MemorySystem::Init();
            sj::ThreadContext::Init(sj::MemorySystem::GetRootMemoryResource(), 256_KiB);

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

            SJ_ENGINE_LOG_INFO("Initializing...");
        }

        ~Engine() = default;

        Window* AddDisplay(const char* title, Vec2 extent)
        {
            s_display = std::make_unique<Window>(title, extent);
            return s_display.get();
        }

        Renderer* AddRenderer(Window* display)
        {
            s_renderer = std::make_unique<Renderer>();
            s_renderer->Init(display);
            return s_renderer.get();
        }

        ECSRegistry* AddECSRegistry(uint32_t initialEntityCapacity)
        {
            s_ecsRegistry = std::make_unique<ECSRegistry>(initialEntityCapacity,
                                                          MemorySystem::GetRootMemoryResource());

            auto registerFn = []<class T>(ECSRegistry& registry) {
                registry.RegisterComponentType<T>();
            };

            g_componentTypes.for_each<registerFn>(*s_ecsRegistry);

            return s_ecsRegistry.get();
        }

        template <class FrameUpdateFn>
            requires std::invocable<FrameUpdateFn, float>
        void Run(FrameUpdateFn&& fn)
        {
            Timer timer;
            auto previousTime = timer.Now();

            while(!Engine::IsGameTerminated())
            {
                float deltaSeconds = timer.Elapsed();
                timer.Reset();

                constexpr float kMaxDeltaTime = 1.0f / 15.0f;
                if(deltaSeconds > kMaxDeltaTime)
                {
                    SJ_ENGINE_LOG_WARN("Large delta time detected- {}. Capping at {}",
                                       deltaSeconds,
                                       kMaxDeltaTime)
                    deltaSeconds = kMaxDeltaTime;
                }

                std::forward<FrameUpdateFn>(fn)(deltaSeconds);
            }
        }

        static Window* GetDisplay()
        {
            return s_display.get();
        }

        static Renderer* GetRenderer()
        {
            return s_renderer.get();
        }

        static ECSRegistry* GetECSRegistry()
        {
            return s_ecsRegistry.get();
        }

        static bool IsGameTerminated()
        {
            return s_display->IsWindowClosed();
        }

    private:
        static std::unique_ptr<Window> s_display;
        static std::unique_ptr<Renderer> s_renderer;
        static std::unique_ptr<ECSRegistry> s_ecsRegistry;
    };
} // namespace sj

namespace sj
{
    std::unique_ptr<Window> Engine::s_display;
    std::unique_ptr<Renderer> Engine::s_renderer;
    std::unique_ptr<ECSRegistry> Engine::s_ecsRegistry;
} // namespace sj