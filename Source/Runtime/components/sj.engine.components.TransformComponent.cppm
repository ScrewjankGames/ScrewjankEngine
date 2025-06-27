module;
#include <ScrewjankDataDefinitions/ComponentMacros.hpp>

export module sj.engine.components.TransformComponent;
import sj.engine.framework.ecs;
import sj.datadefs;
import sj.std.math;

export namespace sj
{
    class TransformComponent2
    {
    public:
        TransformComponent2(const ECSRegistry& registry, const TransformComponentChunk& chunk)
        {
            m_localToParentTransform = chunk.localToParent;

            // GameObjectId parentGoId = registry.FindGameObject(data.parentId);
        }

        SJ_COMPONENT_TYPE(TransformComponent2, TransformComponentChunk);

    private:
        TransformComponent2* m_parent;
        Mat44 m_localToParentTransform = Mat44(kIdentityTag);
    };
} // namespace sj