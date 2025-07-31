module;
#include <ScrewjankEngine/components/ComponentMacros.hpp>

export module sj.engine.ecs.components.TransformComponent;
import sj.datadefs;
import sj.std.math;

export namespace sj
{
    class TransformComponent
    {
    public:
        TransformComponent(const TransformChunk& chunk) 
            : m_localToParentTransform(chunk.localToParent)
        {

            // GameObjectId parentGoId = registry.FindGameObject(data.parentId);
        }

        SJ_COMPONENT(TransformComponent, TransformChunk);

        TransformComponent* m_parent = nullptr;
        Mat44 m_localToParentTransform = Mat44(kIdentityTag);
    };
} // namespace sj