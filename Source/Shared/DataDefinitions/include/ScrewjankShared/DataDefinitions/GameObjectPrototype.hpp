#pragma once

#include <ScrewjankShared/Math/Mat44.hpp>
#include <ScrewjankShared/string/StringHash.hpp>


namespace sj
{
    class GameObjectId
    {
    public:
        GameObjectId() = default;

        GameObjectId(uint32_t sceneId, uint32_t goIndex) 
            : m_sceneId(sceneId), m_goIndex(goIndex)
        {
        }

        static GameObjectId FromInt(uint64_t id)
        {
            uint32_t sceneId = static_cast<uint32_t>(id >> 32);
            uint32_t goId = static_cast<uint32_t>((id << 32) >> 32);
            return 
            {
                sceneId,
                goId
            };
        }

        uint64_t GetInt() const
        {
            return static_cast<uint64_t>((static_cast<uint64_t>(m_sceneId) << 32) | static_cast<uint64_t>(m_goIndex));
        }

        bool operator==(const GameObjectId& other) const
        {
            return other.GetInt() == GetInt();
        }

        bool operator!=(const GameObjectId& other) const
        {
            return !(*this == other);
        }

        std::strong_ordering operator<=>(const GameObjectId& other) const
        {
            return GetInt() <=> other.GetInt();
        }

    private:
        uint32_t m_sceneId = std::numeric_limits<uint32_t>::max();
        uint32_t m_goIndex = std::numeric_limits<uint32_t>::max();
    };

    struct GameObjectPrototype
    {
        GameObjectId id;
        Mat44 transform;
    };

}