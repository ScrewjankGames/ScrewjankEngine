module;

#include <ScrewjankStd/Assert.hpp>
#include <ScrewjankStd/Log.hpp>

#include <SDL3/SDL.h>
#include <SDL3/SDL_events.h>
#include <imgui.h>
#include <imgui_impl_sdl3.h>

#include <concepts>
#include <memory>
#include <string_view>
#include <tuple>

export module sj.engine.core.Program;
export import sj.engine.core.Config;
export import sj.std.type_info;
export import sj.std.signal;

import sj.engine.system.threading.ThreadContext;
import sj.engine.system.memory.MemorySystem;
import sj.engine.system.Timer;

import sj.std.memory.literals;
import sj.std.containers.map;
import sj.std.containers.type_list;
import sj.std.tuple;
import sj.std.type_traits;

export namespace sj
{

template <class... Modules>
class Program;

template <class T, class... Args>
concept Module = requires(T t, float dt) {
    t.Initialize(std::declval<Program<>&>());
    t.NewFrame();
    t.Process(dt);
    t.EndFrame();
};

template <class... Modules>
class Program
{
public:
    signal<void(SDL_KeyboardEvent&)> gKeyboardInputSignal;

    Program(uint64_t rootHeapSize)
    {
        sj::MemorySystem::Init(rootHeapSize);
        sj::ThreadContext::Init(sj::MemorySystem::GetRootMemoryResource(), 256_KiB);

        mConfig = LoadConfig();

        SDL_Init(SDL_INIT_AUDIO | SDL_INIT_VIDEO | SDL_INIT_GAMEPAD);

        new(&mModules) std::tuple<Modules...>();
    }

    ~Program()
    {
        auto destroyModuleFn = []<class T>(T& m) {
            SJ_ENGINE_LOG_INFO("Destroying Module {}", sj::type_name_of<T>);
            std::destroy_at(&m);
        };

        sj::for_each_reverse(mModules, destroyModuleFn);

        SDL_Quit();
    };

    void Start()
    {
        Initialize();
        Run();
    }

    [[nodiscard]] const Config& GetConfig() const
    {
        return mConfig;
    }

    template <class T>
    T* GetModule()
    {
        return &std::get<T>(mModules);
    }

    [[nodiscard]] float GetDeltaSeconds() const
    {
        return mDeltaSeconds;
    }

    [[nodiscard]] std::string_view GetName() const
    {
        return mConfig.program_name;
    }
    
    void EmitEvent(const auto& evt)
    {
        // Visit modules in reverse and allow them to consume events
        [&]<auto... Is>(std::index_sequence<Is...>) {
            auto sendEventFn = [&]<class T>(T& m) -> bool {
                if constexpr(requires { m.ProcessEvent(evt); })
                    return !m.ProcessEvent(evt);
                else
                    return true;
            };

            (sendEventFn(std::get<kNumModules - Is - 1>(mModules)) && ...);
        }(std::index_sequence_for<Modules...> {});
    }

protected:
    void Initialize(this auto&& self)
    {
        auto doInitializeFn = [&]<class T>(T& m) {
            SJ_ENGINE_LOG_INFO("Initializing module: {}", type_name_of<T>);
            m.Initialize(self);
        };

        std::apply(
            [&](auto&... args) {
                ((doInitializeFn(args)), ...);
            },
            self.mModules);
    }

    void Run()
    {
        Timer timer;
        auto previousTime = timer.Now();

        while(!mTerminated)
        {
            mDeltaSeconds = timer.Elapsed();
            if(mDeltaSeconds > kMaxDeltaTime)
            {
                SJ_ENGINE_LOG_WARN("Large delta time detected- {}. Capping at {}",
                                   mDeltaSeconds,
                                   kMaxDeltaTime)
                mDeltaSeconds = kMaxDeltaTime;
            }
            timer.Reset();

            ProcessEvents();

            std::apply(
                [&](auto&... args) {
                    ((args.NewFrame()), ...);
                },
                mModules);

            std::apply(
                [&](auto&... args) {
                    ((args.Process(mDeltaSeconds)), ...);
                },
                mModules);

            std::apply(
                [&](auto&... args) {
                    ((args.EndFrame()), ...);
                },
                mModules);
        }
    }

    void ProcessEvents()
    {
        SDL_Event event;
        while(SDL_PollEvent(&event))
        {
            if(event.type == SDL_EVENT_QUIT)
                mTerminated = true;

            EmitEvent(event);
        }
    }

    static constexpr float kMaxDeltaTime = 1.0f / 15.0f;
    static constexpr size_t kNumModules = std::tuple_size_v<std::tuple<Modules...>>;

    Config mConfig;

    union
    {
        std::tuple<Modules...> mModules;
    };

    bool mTerminated = false;

    float mDeltaSeconds = 0.0f;
};

} // namespace sj