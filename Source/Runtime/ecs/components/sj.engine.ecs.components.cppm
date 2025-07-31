module;

export module sj.engine.ecs.components;
export import sj.engine.ecs.components.TransformComponent;
export import sj.engine.ecs.components.CameraComponent;
export import sj.engine.ecs.components.Mesh3DComponent;
import sj.std.containers.type_list;

export namespace sj
{
    using EngineComponentTypes = type_list<TransformComponent, CameraComponent, Mesh3DComponent>;
    using GameComponentTypes = type_list<>;
    using ComponentTypeList = concat_type_lists<EngineComponentTypes, GameComponentTypes>::list;

    constexpr ComponentTypeList g_componentTypes;
}