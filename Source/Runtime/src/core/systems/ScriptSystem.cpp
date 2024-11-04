// Parent Include
#include <ScrewjankEngine/core/systems/ScriptSystem.hpp>

// Screwjank Engine Includes
#include <ScrewjankEngine/core/Scene.hpp>

namespace sj
{
	void ScriptSystem::Process(Scene* scene, float deltaTime)
	{
        std::span<ScriptComponent> scripts = scene->GetScriptComponents(); 
		
		for(const ScriptComponent& script : scripts)
		{
            script.userScript->Process(deltaTime);
		}
	}
}
