module;

#include <ScrewjankStd/Assert.hpp>

#include <flat_map>

export module sj.std.rtti;
import sj.std.type_info;

export namespace sj::rtti
{
using Lookup = std::flat_map<TypeId, const type_info*>;
Lookup* get_lookup()
{
    static Lookup sRttiLookup;
    return &sRttiLookup;
}

const sj::type_info* find_type_info(TypeId id)
{
    auto it = get_lookup()->find(id);
    if(it == get_lookup()->end())
        return nullptr;

    return it->second;
}

template <class T>
void register_type()
{
    auto res = get_lookup()->try_emplace(type_id_of<T>, &type_info_of<T>);
    SJ_ASSERT(res.second == true,
              "Runtime rtti type id collision. {} and {}",
              type_name_of<T>,
              res.first->second->name);
}

} // namespace sj::rtti