#pragma once

// Engine Headers
#include <ScrewjankEngine/containers/UnorderedMap.hpp>

// Library Headers
#include <SG14/inplace_function.h>

// Shared Headers
#include <ScrewjankShared/string/StringHash.hpp>

namespace sj
{
    using ScriptFactoryFn = stdext::inplace_function<IScriptComponent*(Allocator*)>;

    class ScriptFactory
    {
        void Register(TypeId id, ScriptFactoryFn fn)
        {
            m_callbacks.emplace(id, fn);
        }

        ScriptFactoryFn GetScriptCreateFn(TypeId id)
        {
            return m_callbacks[id];
        }

    private:
        static_unordered_map<TypeId, ScriptFactoryFn, 64> m_callbacks;
    };
}