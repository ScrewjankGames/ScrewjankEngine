module;

export module sj.engine.ecs.components;
export import sj.engine.ecs.components.TransformComponent;
export import sj.engine.ecs.components.CameraComponent;
export import sj.engine.ecs.components.TextureComponent;
import sj.std.containers.type_list;

export namespace sj
{
    using EngineComponentTypes = type_list<TransformComponent, CameraComponent, TextureComponent>;
    using GameComponentTypes = type_list<>;
    using ComponentTypeList = concat_type_lists<EngineComponentTypes, GameComponentTypes>::list;

    constexpr ComponentTypeList g_componentTypes;
}