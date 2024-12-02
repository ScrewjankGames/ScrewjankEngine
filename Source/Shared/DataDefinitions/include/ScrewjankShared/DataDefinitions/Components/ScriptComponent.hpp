#pragma once

// Shared Includes
#include <ScrewjankShared/string/StringHash.hpp>
#include <ScrewjankShared/DataDefinitions/GameObjectPrototype.hpp>
#include <ScrewjankShared/io/File.hpp>

namespace sj
{
    class GameObject;

    class IScriptComponent
    {
    public:
        /**
         *  @param go: The game object your script should operate on
         *  @param deltaTime: game timestep
         */
        virtual void Process(GameObject* go, float deltaTime) {}

        // Need to know how to unpack the user data. First element is always owner game object Id
        virtual void Deserialize(void* userData, uint32_t userDataSize) = 0;
    };

    struct ScriptComponent
    {
        static constexpr uint32_t kTypeId = StringHash("ScriptComponent").AsInt();
        GameObjectId ownerGameobjectId;
        uint32_t scriptTypeId;
        uint32_t userDataSize;

        IScriptComponent* userScript; // Set at load time!
    };
}