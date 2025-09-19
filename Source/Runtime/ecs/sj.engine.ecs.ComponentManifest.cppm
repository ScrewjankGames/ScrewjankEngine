module;

export module sj.engine.ecs.ComponentManifest;
import sj.std.containers.type_list;
import sj.std.string_hash;
import sj.datadefs;
import sj.engine.ecs.components;

export namespace sj
{
    template <class... GameComponentTypes>
    class ComponentManifest
    {
    public:
        using FullComponentTypeList =
            concat_type_lists<EngineComponentTypes, type_list<GameComponentTypes...>>::list;

        constexpr FullComponentTypeList GetComponentTypes()
        {
            return m_componentTypes;
        }

    private:

        static constexpr FullComponentTypeList m_componentTypes = {};
    };
} // namespace sj