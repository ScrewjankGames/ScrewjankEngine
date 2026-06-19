module;

#include <cstdint>
#include <functional>

export module sj.engine.ecs.Identifiers;
import sj.std;

export namespace sj
{
    using GameObjectId = sparse_set_id<uint32_t>;

} // namespace sj

template <>
struct std::hash<sj::GameObjectId>
{
    std::size_t operator()(const sj::GameObjectId& id) const
    {
        uint64_t asInt = static_cast<uint64_t>((static_cast<uint64_t>(id.sparseIndex) << 32) |
                                               static_cast<uint64_t>(id.generation));
        return std::hash<uint64_t>()(asInt);
    }
};
