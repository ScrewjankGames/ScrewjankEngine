#pragma once

namespace sj
{
    // Forward Declarations
    class Scene;

    class ScriptSystem
    {
    public:
        ScriptSystem() = default;
        ~ScriptSystem() = default;

        void Process(Scene* scene, float deltaTime);
    };
}