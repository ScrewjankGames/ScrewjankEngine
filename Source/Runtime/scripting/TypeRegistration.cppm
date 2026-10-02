module;

#include <lua.h>
#include <lualib.h>
#include <LuaBridge/LuaBridge.h>
#include <LuaBridge/detail/LuaRef.h>

export module sj.engine.scripting:TypeRegistration;
import :LuauStack;

import sj.engine.InputSystem;
import sj.engine.TransformComponent;

import sj.std;
import sj.engine.ecs;

export namespace sj
{

template <size_t tRow>
void mat44SetterHelper(mat44* m, const vec4& v)
{
    m->set_row<tRow>(v);
}

void RegisterTypes(lua_State* L)
{
    luabridge::getGlobalNamespace(L)
        .beginClass<mat44>("mat44")
        .addProperty("x", &mat44::get_row<0>, mat44SetterHelper<0>)
        .addProperty("y", &mat44::get_row<1>, mat44SetterHelper<1>)
        .addProperty("z", &mat44::get_row<2>, mat44SetterHelper<2>)
        .addProperty("w", &mat44::get_row<3>, mat44SetterHelper<3>)
        .addFunction("get_euler_angles", &mat44::get_euler_angles)
        .addFunction("set_rot_euler_xyz", &mat44::set_rot_euler_xyz)
        .endClass()

        .beginClass<GameObject>("GameObject")
        .addProperty(
            "TransformWS",
            [](GameObject& go) -> mat44 {
                return go.GetTransformLW();
            },
            [](GameObject& go, const mat44& ws) {
                go.SetTransformLW(ws);
            })
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