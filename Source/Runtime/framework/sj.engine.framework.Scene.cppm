module;

// Shared Includes
#include <ScrewjankStd/TypeMacros.hpp>
#include <ScrewjankShared/io/File.hpp>
#include <ScrewjankStd/Assert.hpp>

// Library Includes
#include <glaze/beve/read.hpp>

export module sj.engine.framework.Scene;
import sj.engine.framework.ecs;
import sj.std.memory;
import sj.std.containers;
import sj.shared.datadefs;

export namespace sj
{
    using CreateComponentFn = void (*)(ECSRegistry& registry,
                                       GameObjectId goId,
                                       const AnyComponent& component);
    using ReificationFnLookup =
        static_flat_map<sj::TypeId, CreateComponentFn, ComponentTypeRegistry::kNumComponentTypes>;
    constexpr const ReificationFnLookup& GetReificationFunctions()
    {
        auto makeLookupFn = []() -> ReificationFnLookup {
            ReificationFnLookup tmpLookup;

            auto registerFn = []<class T>(ReificationFnLookup& tmpLookup) {
                auto reifyFn =
                    [](ECSRegistry& registry, GameObjectId goId, const AnyComponent& component) {
                        registry.CreateComponent<T>(goId, component.get<T>());
                    };

                tmpLookup.emplace(T::kTypeId, reifyFn);
            };

            ComponentTypeRegistry::ForEachComponentType<registerFn>(tmpLookup);

            return tmpLookup;
        };

        static ReificationFnLookup lookup = makeLookupFn();
        return lookup;
    }

    class Scene
    {
    public:
        Scene(ECSRegistry& registry, const char* path)
        {
            glz::error_ctx errorCtx =
                glz::read_file_beve(m_chunk, path, sj::dynamic_vector<std::byte> {});
            SJ_ASSERT(errorCtx.ec == glz::error_code::none, "Failed to load scene {0}", path);

            const ReificationFnLookup& reificationFunctions = GetReificationFunctions();
            for(const GameObjectChunk& goChunk : m_chunk.gameObjects)
            {
                GameObjectId goId = registry.CreateGameObject();
                (void)goId;
                for(AnyChunk& component : goChunk.components)
                {
                    const ComponentMetaInfo* info =
                        sj::ComponentTypeRegistry::FindComponentMetaInfo(component.type);
                    SJ_ASSERT(info != nullptr, "Failed to find reflection info for component typeId");

                    auto res = info->m_serializationFuncs.fromBeveFn(component.data);

                    if(res.has_value())
                    {
                        const auto& reificationEntry = reificationFunctions.find(component.type);
                        SJ_ASSERT(reificationEntry != reificationFunctions.end(), "Failed to find reification function for component type" );
                        std::invoke(reificationEntry->second, registry, goId, *res);
                    }
                    else
                    {
                        SJ_ASSERT(false, "Unhandled glaze exception!");
                    }
                }
            }
        }

        ~Scene() = default;

    private:
        SceneChunk m_chunk;
        free_list_allocator m_memoryResource;
    };
} // namespace sj