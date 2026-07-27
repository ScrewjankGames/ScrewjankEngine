module;

#include <ScrewjankStd/Assert.hpp>

#include <lua.h>
#include <lualib.h>
#include <LuaBridge/LuaBridge.h>
#include <LuaBridge/detail/LuaRef.h>

export module sj.engine.scripting:Signal;
import sj.std;

export namespace sj
{
template <class... Args>
class LuauSignal
{
public:
    // Store the callback using luabridge::LuaRef
    void Connect(luabridge::LuaRef callback)
    {
        SJ_ASSERT(callback.isFunction(), "Can only connect function to luau signals");
        slots.emplace_back(std::move(callback));
    }

    void Trigger(Args&... args)
    {
        for(luabridge::LuaRef& slot : slots)
            slot(args...);
    }

private:
    std::vector<luabridge::LuaRef> slots;
};

using ProcessSignal = LuauSignal<float>;

} // namespace sj