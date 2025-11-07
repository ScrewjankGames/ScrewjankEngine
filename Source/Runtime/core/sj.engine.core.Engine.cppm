module;

#include <ScrewjankStd/Assert.hpp>
#include <ScrewjankStd/Log.hpp>

#include <memory>
#include <concepts>
#ifndef SJ_VERSION
    #include <imgui.h>
#endif

export module sj.engine.core.Engine;
import sj.engine.system.threading.ThreadContext;
import sj.engine.system.memory.MemorySystem;
import sj.engine.system.Timer;
import sj.std.memory.literals;

export namespace sj
{
    class Engine
    {
    public:
        Engine(uint64_t rootHeapSize)
        {
            sj::MemorySystem::Init(rootHeapSize);
            sj::ThreadContext::Init(sj::MemorySystem::GetRootMemoryResource(), 256_KiB);

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