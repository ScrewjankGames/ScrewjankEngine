#include <cstdint>

import sj.std.string_hash;

namespace sj
{
}

#define SJ_STRUCT_TYPE_ID(type)                                                                    \
    static constexpr sj::TypeId kTypeId = sj::string_hash(#type).AsInt()

