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

export module sj.engine.Program;
export import sj.engine.config;
export import sj.std.type_info;

import sj.std;
import sj.datadefs;

import sj.engine.system.threading.ThreadContext;
import sj.engine.system.memory.MemorySystem;

export namespace sj
{

template <class... Modules>
class Program
{
public:
    Program(uint64_t rootHeapSize)
    {
        sj::MemorySystem::Init(rootHeapSize);
        sj::ThreadContext::Init(sj::MemorySystem::GetRootMemoryResource(), 256_KiB);

        mConfig = LoadConfig();
        mAssetDB.Load("Data/.AssetDB");

        bool success = SDL_Init(SDL_INIT_VIDEO | SDL_INIT_JOYSTICK | SDL_INIT_GAMEPAD);
        if(!success)
        {
            const char* err = SDL_GetError();
            SJ_ASSERT(false, "Failed to initialize SDL: {}", err);
            mTerminated = true;
        }

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

    [[nodiscard]] const AssetDB& GetAssetDB() const
    {
        return mAssetDB;
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
        template for(auto&& m : self.mModules)
        {
            m.initialize(self);
        }
    }

    void Run()
    {
        timer timer;
        auto previousTime = timer.now();

        while(!mTerminated)
        {
            mDeltaSeconds = timer.elapsed();
            if(mDeltaSeconds > kMaxDeltaTime)
            {
                SJ_ENGINE_LOG_WARN("Large delta time detected- {}. Capping at {}",
                                   mDeltaSeconds,
                                   kMaxDeltaTime)
                mDeltaSeconds = kMaxDeltaTime;
            }
            timer.reset();

            ProcessEvents();

            template for (auto&& m : mModules)
            {
                m.NewFrame();
            }

            template for (auto&& m : mModules)
            {
                m.Process(mDeltaSeconds);
            }

            template for (auto&& m : mModules)
            {
                m.EndFrame();
            }
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

    AssetDB mAssetDB;
    Config mConfig;

    union
    {
        std::tuple<Modules...> mModules;
    };

    bool mTerminated = false;

    float mDeltaSeconds = 0.0f;
};

} // namespace sj