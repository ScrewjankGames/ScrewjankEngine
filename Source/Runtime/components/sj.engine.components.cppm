module;

export module sj.engine.components;
export import sj.engine.components.TransformComponent;
export import sj.engine.components.CameraComponent;
import sj.std.containers.type_list;

export namespace sj
{
    using EngineComponentTypes = type_list<TransformComponent, CameraComponent>;
    using GameComponentTypes = type_list<>;
    using ComponentTypeList = concat_type_lists<EngineComponentTypes, GameComponentTypes>::list;

    constexpr ComponentTypeList g_componentTypes;
}