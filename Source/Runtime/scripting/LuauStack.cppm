module;


#include <lua.h>
#include <lualib.h>
#include <LuaBridge/LuaBridge.h>
#include <LuaBridge/detail/LuaRef.h>

export module sj.engine.scripting:LuauStack;
import sj.std;

export namespace luabridge
{
template <>
struct Stack<sj::vec4>
{
    static Result push(lua_State* L, const sj::vec4& vec)
    {
        // Push as a native vector (Luau's internal vector type)
        lua_pushvector(L, vec.get_x(), vec.get_y(), vec.get_z(), vec.get_w());
        return Result {};
    }

    static TypeResult<sj::vec4> get(lua_State* L, int index)
    {
        // Retrieve Luau vector from the stack
        const float* vec = lua_tovector(L, index);
        return sj::vec4(vec[0], vec[1], vec[2], vec[3]);
    }

    static bool isInstance(lua_State* L, int index)
    {
        return lua_isvector(L, index);
    }
};

template <>
struct Stack<sj::vec3>
{
    static Result push(lua_State* L, const sj::vec4& vec)
    {
        // Push as a native vector (Luau's internal vector type)
        lua_pushvector(L, vec.get_x(), vec.get_y(), vec.get_z(), 0.0f);
        return Result {};
    }

    static TypeResult<sj::vec3> get(lua_State* L, int index)
    {
        // Retrieve Luau vector from the stack
        const float* vec = lua_tovector(L, index);
        return sj::vec3(vec[0], vec[1], vec[2]);
    }

    static bool isInstance(lua_State* L, int index)
    {
        return lua_isvector(L, index);
    }
};
} // namespace luabridge