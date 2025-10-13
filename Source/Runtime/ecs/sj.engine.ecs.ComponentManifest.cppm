module;

export module sj.engine.ecs.ComponentManifest;
import sj.std.containers.type_list;
import sj.std.string_hash;
import sj.datadefs;

export namespace sj
{
template <class... GameComponentTypes>
class ComponentManifest
{
public:
    using FullComponentTypeList = type_list<GameComponentTypes...>;

    constexpr FullComponentTypeList GetComponentTypes()
    {
        return m_componentTypes;
    }

private:
    static constexpr FullComponentTypeList m_componentTypes = {};
};
} // namespace sj