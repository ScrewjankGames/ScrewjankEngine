#pragma once

// Engine Headers
#include <ScrewjankEngine/containers/UnorderedMap.hpp>
#include <ScrewjankEngine/system/memory/Allocator.hpp>

// Library Headers
#include <SG14/inplace_function.h>

// Shared Headers
#include <ScrewjankShared/string/StringHash.hpp>

namespace sj
{
    using ScriptFactoryFn = stdext::inplace_function<IScriptComponent*(Allocator*)>;

    class ScriptFactory
    {
    public:
        void Register(TypeId id, ScriptFactoryFn fn)
        {
            m_callbacks.emplace(id, fn);
        }


        const ScriptFactoryFn* GetScriptCreateFn(TypeId id) const
        {
            const auto& it = m_callbacks.find(id);

            if(it == m_callbacks.end())
                return nullptr;
            else
                return &(it->second);
        }

    private:
        static_unordered_map<TypeId, ScriptFactoryFn, 64> m_callbacks;
    };
}