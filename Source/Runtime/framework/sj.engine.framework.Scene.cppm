module;

// SJ Includes
#include <ScrewjankStd/TypeMacros.hpp>
#include <ScrewjankStd/Assert.hpp>

// Library Includes
#include <glaze/beve/read.hpp>

export module sj.engine.framework.Scene;
import sj.engine.framework.ecs;
import sj.std.memory;
import sj.std.containers;
import sj.datadefs;

export namespace sj
{
    using CreateComponentFn = void (*)(ECSRegistry& registry,
                                       GameObjectId goId,
                                       const AnyComponent& component);

    template<class T>
    void CreateComponent(ECSRegistry& registry, GameObjectId goId, const AnyComponent& component) 
    {
        registry.CreateComponent<T>(goId, component.get<T>());
    };

    constexpr type_map<
        g_componentTypes, 
        TypeId,
        CreateComponentFn,
        []<class T>() -> TypeId{ return T::kTypeId; }, 
        []<class T>() -> CreateComponentFn { return CreateComponent<T>; } > 
        kComponentCreateLookup = {};

    class Scene
    {
    public:
        Scene(ECSRegistry& registry, const char* path)
        {
            glz::error_ctx errorCtx =
                glz::read_file_beve(m_chunk, path, sj::dynamic_vector<std::byte> {});
            SJ_ASSERT(errorCtx.ec == glz::error_code::none, "Failed to load scene {0}", path);

            for(const GameObjectChunk& goChunk : m_chunk.gameObjects)
            {
                GameObjectId goId = registry.CreateGameObject();
                for(DataChunk& component : goChunk.components)
                {
                    const ComponentMetaInfo* info = g_componentSerializationFuncs.find(component.type);
                    SJ_ASSERT(info != nullptr, "Failed to find reflection info for component typeId");

                    auto res = info->m_serializationFuncs.fromBeveFn(component.data);

                    if(res.has_value())
                    {
                        CreateComponentFn createFn = kComponentCreateLookup.get(component.type);
                        std::invoke(createFn, registry, goId, *res);
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