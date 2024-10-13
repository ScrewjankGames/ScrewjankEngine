#pragma once

// Shared Includes
#include <ScrewjankShared/string/StringHash.hpp>
#include <ScrewjankShared/DataDefinitions/GameObjectPrototype.hpp>

namespace sj
{
    struct ScriptComponentPrototype
    {
        static constexpr uint32_t kTypeId = StringHash("ScriptComponentPrototype").AsInt();
        GameObjectId ownerGameobjectId;
        uint32_t scriptTypeId;
        uint32_t userDataSize;
        uint8_t* userData; // User data blob starts here and extends past end of struct
    };

    class IScriptComponent
    {
    public:
        GameObjectId ownerGameobjectId;

        virtual void Process(float deltaTime) {}

        // Need to know how to unpack the user data. First element is always owner game object Id
        virtual void Deserialize(void* userData, uint32_t userDataSize) = 0;
    };
}