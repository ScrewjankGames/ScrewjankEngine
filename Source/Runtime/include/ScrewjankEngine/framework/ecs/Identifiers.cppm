module;

#include <cstdint>
#include <functional>

export module sj.engine.framework.ecs:Identifiers;
import sj.shared.containers;

export namespace sj
{
    struct ComponentId
    {
        using index_type = uint32_t;

        index_type sparseIndex;
        index_type generation;
    };

    struct GameObjectId
    {
        using index_type = uint32_t;

        index_type sparseIndex;
        index_type generation;
    };
}

template <>
struct std::hash<sj::GameObjectId>
{
    std::size_t operator()( const sj::GameObjectId& id ) const
    {
        uint64_t asInt = static_cast<uint64_t>((static_cast<uint64_t>(id.sparseIndex) << 32) | static_cast<uint64_t>(id.generation));
        return std::hash<uint64_t>()(asInt);
    }
};
