module;

#include <ScrewjankStd/Assert.hpp>
#include <ScrewjankStd/Log.hpp>

#include <SDL3/SDL.h>

#include <concepts>
#include <memory>
#include <string_view>

#ifndef SJ_VERSION
    #include <imgui.h>
#endif

export module sj.engine.core.Program;
import sj.engine.system.threading.ThreadContext;
import sj.engine.system.memory.MemorySystem;
import sj.engine.system.Timer;

import sj.std.type_info;
import sj.std.memory.literals;
import sj.std.containers.unmanaged_list;

export namespace sj
{

class Program;

template <class T, class... Args>
concept Module = requires(T t) {
    T(std::declval<Program&>(), std::declval<Args>()...); // Accepts program as first ctor arg
};

class IModule : public dl_list_node<IModule>
{
public:
    TypeId mTypeId = {};
    virtual ~IModule() = default;

    virtual void NewFrame()
    {
    }

    virtual void Process([[maybe_unused]] float deltaSeconds)
    {
    }

    virtual void EndFrame()
    {
    }
};

class Program
{
public:
    Program(std::string_view programName, uint64_t rootHeapSize) : mProgramName(programName)
    {
        sj::MemorySystem::Init(rootHeapSize);
        sj::ThreadContext::Init(sj::MemorySystem::GetRootMemoryResource(), 256_KiB);
        
        SDL_Init(SDL_INIT_AUDIO | SDL_INIT_VIDEO | SDL_INIT_GAMEPAD);

        SJ_ENGINE_LOG_INFO("Initializing...");
    }

    ~Program()
    {
        SDL_Quit();
    };

    template <class T, class... Args>
        requires Module<T, Args...>
    T* AddModule(Args&&... args)
    {
        static T sModule(*this, std::forward<Args>(args)...);
        sModule.mTypeId = type_id_of<T>;
        mModules.push_back(&sModule);
        return &sModule;
    }

    template <class T>
    T* GetModule()
    {
        auto it = std::ranges::find(mModules, type_id_of<T>, &IModule::mTypeId);
        SJ_ASSERT(it != mModules.end(), "Failed to find module {}", type_name_of<T>);

        return static_cast<T*>(&*it);
    }

    void Run()
    {
        Timer timer;
        auto previousTime = timer.Now();

        while(!mTerminated)
        {
            mDeltaSeconds = timer.Elapsed();
            timer.Reset();

            ProcessEvents();

            constexpr float kMaxDeltaTime = 1.0f / 15.0f;
            if(mDeltaSeconds > kMaxDeltaTime)
            {
                SJ_ENGINE_LOG_WARN("Large delta time detected- {}. Capping at {}",
                                   mDeltaSeconds,
                                   kMaxDeltaTime)
                mDeltaSeconds = kMaxDeltaTime;
            }

            for(IModule& m : mModules)
                m.NewFrame();

            for(IModule& m : mModules)
                m.Process(mDeltaSeconds);

            for(IModule& m : mModules)
                m.EndFrame();
        }
    }

    [[nodiscard]] float GetDeltaSeconds() const
    {
        return mDeltaSeconds;
    }

    [[nodiscard]] std::string_view GetName() const
    {
        return mProgramName;
    }

private:

    void ProcessEvents()
    {
        SDL_Event event;
        while(SDL_PollEvent(&event))
        {
            if(event.type == SDL_EVENT_QUIT)
            {
                mTerminated = true;
            }
        }
    }


    std::string_view mProgramName;
    unmanaged_list<IModule> mModules;
    bool mTerminated = false;
    float mDeltaSeconds = 0.0f;
};

} // namespace sj