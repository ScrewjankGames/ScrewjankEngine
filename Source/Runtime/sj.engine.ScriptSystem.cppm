module;

#include <ScrewjankStd/Assert.hpp>

#include <fstream>
#include <lua.h>
#include <lualib.h>
#include <LuaBridge/LuaBridge.h>
#include <LuaBridge/detail/LuaRef.h>

#include <flat_map>
#include <optional>
#include <string_view>

export module sj.engine.ScriptSystem;
import sj.std;
import sj.datadefs;
import sj.engine.ecs;
import sj.engine.TransformComponent;
import sj.engine.system.threading.ThreadContext;

namespace luabridge
{
template <>
struct Stack<sj::Vec4>
{
    static Result push(lua_State* L, const sj::Vec4& vec)
    {
        // Push as a native vector (Luau's internal vector type)
        lua_pushvector(L, vec.GetX(), vec.GetY(), vec.GetZ(), vec.GetW());
        return Result {};
    }

    static TypeResult<sj::Vec4> get(lua_State* L, int index)
    {
        // Retrieve Luau vector from the stack
        const float* vec = lua_tovector(L, index);
        return sj::Vec4(vec[0], vec[1], vec[2], vec[3]);
    }

    static bool isInstance(lua_State* L, int index)
    {
        return lua_isvector(L, index);
    }
};
} // namespace luabridge

export namespace sj
{
struct ScriptComponent
{
    AssetID script_id;
};

class ScriptSystem
{
public:
    static constexpr type_list<ScriptComponent> kRegisteredComponents;
    static constexpr type_list<ScriptComponent> kRequestedLifetimeCallbacks;

    ScriptSystem() : L(luaL_newstate())
    {
        luaL_openlibs(L);
        SetupEnv(L);
    }

    ~ScriptSystem()
    {
        lua_close(L);
    }

    void Initialize(const AssetDB* adb, ECSRegistry* ecs)
    {
        mAssetDB = adb;
        mEcs = ecs;
    }

    void Process(ECSRegistry& ecs, float deltaTime)
    {
        mProcessCallbacks.Trigger(deltaTime);
    }

    void OnCreate(GameObjectId goId, ScriptComponent* component)
    {
        auto scriptIt = mScripts.find(component->script_id);
        if(scriptIt == mScripts.end())
            mScripts.emplace(component->script_id, LoadScript(component->script_id));

        scriptIt->second.refcount_increment();

        [[maybe_unused]] luabridge::Result res = luabridge::push(L, GameObject(goId, mEcs));
        scriptIt->second->call(luabridge::LuaRef::fromStack(L));
    }

    void OnDestroy(GameObjectId goId, ScriptComponent* component)
    {
        auto scriptIt = mScripts.find(component->script_id);
        scriptIt->second.refcount_decrement();
    }

private:
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

    template <size_t tRow>
    static void Mat44SetterHelper(Mat44* m, const Vec4& v)
    {
        m->SetRow<tRow>(v);
    }

    void SetupEnv(lua_State* L)
    {
        luabridge::getGlobalNamespace(L)
            .beginClass<Mat44>("Mat44")
            .addProperty("x", &Mat44::GetRow<0>, Mat44SetterHelper<0>)
            .addProperty("y", &Mat44::GetRow<1>, Mat44SetterHelper<1>)
            .addProperty("z", &Mat44::GetRow<2>, Mat44SetterHelper<2>)
            .addProperty("w", &Mat44::GetRow<3>, Mat44SetterHelper<3>)
            .endClass()

            .beginClass<GameObject>("GameObject")
            .addFunction("GetTransformLw",
                         [](GameObject& go) -> Mat44& {
                             return go.GetComponent<TransformComponent>()->localToParent;
                         })
            .endClass()

            .beginClass<ProcessSignal>("ProcessSignal")
            .addFunction("Connect", &ProcessSignal::Connect)
            .endClass();

        luabridge::getGlobalNamespace(L)
            .beginNamespace("Game")
            .addVariable("Process", &mProcessCallbacks)
            .endNamespace();
    }

    luabridge::LuaRef LoadScript(AssetID id)
    {
        std::string_view scriptPath = mAssetDB->GetAssetPath(id);

        std::ifstream file(scriptPath.data(), std::ios::binary | std::ios::ate);
        SJ_ASSERT(file.is_open(), "Unable to load script component- file not found");
        std::streamsize size = file.tellg();
        file.seekg(0, std::ios::beg);

        auto scope = ThreadContext::GetScratchpad();
        std::pmr::vector<char> bytecode(&scope.get_allocator());
        bytecode.reserve(size);
        file.read(bytecode.data(), size);

        std::pmr::string chunkName(&scope.get_allocator());
        chunkName = scriptPath;

        luau_load(L, chunkName.c_str(), bytecode.data(), bytecode.size(), 0);
        return luabridge::LuaRef::fromStack(L, -1);
    }

    lua_State* L;
    ProcessSignal mProcessCallbacks;

    std::flat_map<AssetID, ref<luabridge::LuaRef>> mScripts;

    const AssetDB* mAssetDB = nullptr;
    ECSRegistry* mEcs = nullptr;
};

} // namespace sj