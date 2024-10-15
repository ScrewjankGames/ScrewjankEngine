// Parent Include
#include <ScrewjankEngine/core/systems/ScriptSystem.hpp>

// Screwjank Engine Includes
#include <ScrewjankEngine/core/Scene.hpp>

namespace sj
{
	void ScriptSystem::Process(Scene* scene, float deltaTime)
	{
        const ScriptDatabase& scripts = scene->GetScriptComponents(); 
		
		for(const auto& entry : scripts)
		{
            for(std::unique_ptr<IScriptComponent>& component : entry.second)
			{
                component->Process(deltaTime);
			}
		}
	}
}
