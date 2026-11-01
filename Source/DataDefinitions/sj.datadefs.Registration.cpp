module sj.datadefs.Registration;
import sj.datadefs.components.TransformComponent;
import sj.datadefs.components.DirectionalLightComponent;
import sj.datadefs.components.Mesh3DComponent;
import sj.datadefs.components.PhysicsComponents;
import sj.datadefs.components.CameraComponent;
import sj.datadefs.components.ScriptComponent;

import sj.std.rtti;

namespace sj::datadefs
{
TypeInfoLookup* GetLookup()
{
    static TypeInfoLookup sLookup;
    return &sLookup;
}

const DatadefTypeInfo* FindTypeInfo(TypeId id)
{
    auto it = GetLookup()->find(id);
    if(it == GetLookup()->end())
        return nullptr;
    return it->second;
}

void RegisterEngineComponentTypes()
{
    RegisterComponentType<CameraComponent>();
    RegisterComponentType<DirectionalLightComponent>();
    RegisterComponentType<Mesh3DComponent>();
    RegisterComponentType<RigidbodyComponent>();
    RegisterComponentType<BoxShape>();
    RegisterComponentType<SphereShape>();
    RegisterComponentType<TransformComponent>();
    RegisterComponentType<ScriptComponent>();
}
} // namespace sj::datadefs