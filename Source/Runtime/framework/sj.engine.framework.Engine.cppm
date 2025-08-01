module;

#include <ScrewjankStd/Assert.hpp>

#include <string_view>

export module sj.engine.framework.Engine;
import sj.engine.rendering.Renderer;
import sj.engine.ecs;

export namespace sj
{
    /**
     * Service locator to provide global access to engine systems
     */
    class Engine
    {
        public:
        
            static void RegisterGameName(std::string_view name)
            {
                s_gameName = name;
            }

            static std::string_view GetGameName()
            {
                return s_gameName;
            }

            static void RegisterRenderer(Renderer* renderer)
            {
                SJ_ASSERT(s_renderer == nullptr, "Double registration detected");
                s_renderer = renderer;
            }

            static auto GetRenderer() -> Renderer*
            {
                return s_renderer;
            }

            static void RegisterECSRegistry(ECSRegistry* registry)
            {
                SJ_ASSERT(s_ecsRegistry == nullptr, "Double registration detected");
                s_ecsRegistry = s_ecsRegistry;
            }

            static auto GetECSRegistry() -> ECSRegistry*
            {
                return s_ecsRegistry;
            }

        private:
            static std::string_view s_gameName;
            static Renderer* s_renderer;
            static ECSRegistry* s_ecsRegistry;
    };
}

namespace sj
{
    std::string_view Engine::s_gameName = "";
    Renderer* Engine::s_renderer = nullptr;
    ECSRegistry* Engine::s_ecsRegistry = nullptr;
}