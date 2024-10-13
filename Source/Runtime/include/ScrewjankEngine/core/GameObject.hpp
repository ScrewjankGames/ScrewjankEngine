#pragma once

// Engine Incudes
#include <ScrewjankEngine/containers/List.hpp>

// Shared Includes
#include <ScrewjankShared/DataDefinitions/GameObjectPrototype.hpp>
#include <ScrewjankShared/string/StringHash.hpp>
#include <ScrewjankShared/Math/Mat44.hpp>

namespace sj
{
	class GameObject
	{
    public:
        GameObject() = default;
        GameObject(const GameObjectPrototype& proto);

        GameObjectId GetGoId() const { return m_id; }
        
        const Mat44& GetWorldSpaceTransform() const { return m_transform; }

    private:
        GameObjectId m_id;
        Mat44 m_transform;
	};
}