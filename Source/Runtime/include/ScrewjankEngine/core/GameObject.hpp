#pragma once

// Engine Incudes
#include <ScrewjankEngine/containers/List.hpp>

// Shared Includes
#include <ScrewjankShared/string/StringHash.hpp>
#include <ScrewjankShared/Math/Mat44.hpp>

namespace sj
{
	class GameObject
	{

    private:
        Mat44 m_transform;

        StringHash m_nameHash;
        uint32_t m_id;
	};
}