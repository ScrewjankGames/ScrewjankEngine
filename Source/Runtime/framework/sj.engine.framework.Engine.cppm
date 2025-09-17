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
import sj.std.memory.literals;
import sj.std.math;
import sj.std.string_hash;
import sj.std.containers.vector;

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

        template <class TerminateFn, class FrameUpdateFn>
            requires std::invocable<FrameUpdateFn, float>
        void Run(TerminateFn&& terminateFn, FrameUpdateFn&& updateFn)
        {
            Timer timer;
            auto previousTime = timer.Now();

            while(!std::forward<TerminateFn>(terminateFn)())
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

                std::forward<FrameUpdateFn>(updateFn)(deltaSeconds);
            }
        }
    };
} // namespace sj