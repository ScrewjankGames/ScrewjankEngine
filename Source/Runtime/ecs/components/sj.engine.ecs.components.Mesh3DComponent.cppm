module;
#include <ScrewjankEngine/components/ComponentMacros.hpp>

export module sj.engine.ecs.components.Mesh3DComponent;
import sj.datadefs;
import sj.std.math;

export namespace sj
{
    class Mesh3DComponent
    {
    public:
        Mesh3DComponent(const Mesh3DChunk& chunk) 
        {
            // GameObjectId parentGoId = registry.FindGameObject(data.parentId);
        }

        SJ_COMPONENT(Mesh3DComponent, Mesh3DChunk);

    };
} // namespace sj