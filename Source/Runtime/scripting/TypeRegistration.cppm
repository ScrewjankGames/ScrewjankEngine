module;

#include <lua.h>
#include <lualib.h>
#include <LuaBridge/LuaBridge.h>
#include <LuaBridge/detail/LuaRef.h>

export module sj.engine.scripting:TypeRegistration;
import :LuauStack;
import :Signal;

import sj.engine.InputSystem;
import sj.engine.TransformComponent;

import sj.std;
import sj.engine.ecs;

export namespace sj
{

template <size_t tRow>
vec3 mat44GetterHelper(const mat44* m)
{
    const vec4& v = m->get_row<tRow>();
    return vec3(v.get_x(), v.get_y(), v.get_z());
}

template <size_t tRow>
void mat44SetterHelper(mat44* m, const vec3& v)
{
    if constexpr(tRow == 3)
        m->set_row<tRow>(vec4(v, 1.0f));
    else
        m->set_row<tRow>(vec4(v, 0.0f));
}

void RegisterTypes(lua_State* L)
{
    luabridge::getGlobalNamespace(L)
        .beginClass<mat44>("transform")
        .addProperty("x", mat44GetterHelper<0>, mat44SetterHelper<0>)
        .addProperty("y", mat44GetterHelper<1>, mat44SetterHelper<1>)
        .addProperty("z", mat44GetterHelper<2>, mat44SetterHelper<2>)
        .addProperty("w", mat44GetterHelper<3>, mat44SetterHelper<3>)
        .addFunction("get_euler_angles", &mat44::get_euler_angles)
        .addFunction("set_rot_euler_xyz", &mat44::set_rot_euler_xyz)
        .endClass()

        .beginClass<GameObject>("GameObject")
        .addFunction("get_transform_ws",
                     [](GameObject& go) -> mat44 {
                         return go.GetTransformLW();
                     })
        .addFunction("set_transform_ws",
                     [](GameObject& go, const mat44& ws) {
                         go.SetTransformLW(ws);
                     })
        .endClass()

        .beginClass<ProcessSignal>("ProcessSignal")
        .addFunction("Connect", &ProcessSignal::Connect)
        .endClass()

        .beginClass<InputSystem>("InputSystem")
        .addFunction("GetAxisValue",
                     [](InputSystem* input, const std::string_view& str) -> float {
                         return input->GetAxisValue(str);
                     })
        .addFunction("GetAxisValue2D",
                     [](InputSystem* input, const std::string_view& str) -> vec4 {
                         return input->GetAxisValue2D(str);
                     })
        .endClass();
}
} // namespace sj