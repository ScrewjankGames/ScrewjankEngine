module;

#include <ScrewjankStd/Assert.hpp>

#include <fstream>
#include <lua.h>
#include <lualib.h>
#include <LuaBridge/LuaBridge.h>
#include <LuaBridge/detail/LuaRef.h>

#include <flat_map>
#include <memory>
#include <string_view>

export module sj.engine.ScriptSystem;
import sj.std;
import sj.datadefs;
import sj.engine.ecs;
import sj.engine.TransformComponent;
import sj.engine.system.threading.ThreadContext;
import sj.engine.InputSystem;

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

template <>
struct Stack<sj::Vec3>
{
    static Result push(lua_State* L, const sj::Vec4& vec)
    {
        // Push as a native vector (Luau's internal vector type)
        lua_pushvector(L, vec.GetX(), vec.GetY(), vec.GetZ(), 0.0f);
        return Result {};
    }

    static TypeResult<sj::Vec3> get(lua_State* L, int index)
    {
        // Retrieve Luau vector from the stack
        const float* vec = lua_tovector(L, index);
        return sj::Vec3(vec[0], vec[1], vec[2]);
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

    ScriptSystem()
        : L(luaL_newstate(), [](lua_State* l) {
              lua_close(l);
          })
    {
        luaL_openlibs(L.get());
    }

    ~ScriptSystem() = default;

    void Initialize(const AssetDB* adb, ECSRegistry* ecs, InputSystem* input)
    {
        mAssetDB = adb;
        mEcs = ecs;
        mInputSystem = input;
        SetupEnv(L.get());
    }

    void Process(ECSRegistry& ecs, float deltaTime)
    {
        mProcessCallbacks.Trigger(deltaTime);
    }

    void OnCreate(GameObjectId goId, ScriptComponent* component)
    {
        AssetID scriptId = component->script_id;

        auto scriptIt = mScripts.find(scriptId);
        if(scriptIt == mScripts.end())
            scriptIt =
                mScripts.emplace(scriptId, LoadScript(L.get(), mAssetDB->GetAssetPath(scriptId)))
                    .first;

        scriptIt->second.refcount_increment();

        [[maybe_unused]] luabridge::Result res = luabridge::push(L.get(), GameObject(goId, mEcs));
        scriptIt->second->call(luabridge::LuaRef::fromStack(L.get()));
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

    static luabridge::LuaRef LoadScript(lua_State* L, std::string_view scriptPath)
    {
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
        luabridge::LuaRef res = luabridge::LuaRef::fromStack(L, -1);
        return res;
    }

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
            .addFunction("GetEulerAngles", &Mat44::GetEulerAngles)
            .addFunction("SetRotationEulerXYZ", &Mat44::SetRotationEulerXYZ)
            .endClass()

            .beginClass<GameObject>("GameObject")
            .addProperty(
                "TransformWS",
                [](GameObject& go) -> Mat44 {
                    return go.GetComponent<TransformComponent>()->localToParent;
                },
                [](GameObject& go, const Mat44& ws) {
                    go.GetComponent<TransformComponent>()->localToParent = ws;
                })
            .endClass()

            .beginClass<InputSystem>("InputSystem")
            .addFunction("GetAxisValue",
                         [](InputSystem* input, const std::string_view& str) -> float {
                             return input->GetAxisValue(str);
                         })
            .endClass()

            .beginClass<ProcessSignal>("ProcessSignal")
            .addFunction("Connect", &ProcessSignal::Connect)
            .endClass();

        luabridge::getGlobalNamespace(L)
            .beginNamespace("Game")
            .addVariable("Process", &mProcessCallbacks)
            .addVariable("InputSystem", mInputSystem)
            .endNamespace();
    }

    std::unique_ptr<lua_State, void (*)(lua_State*)> L;
    ProcessSignal mProcessCallbacks;
    std::flat_map<AssetID, ref<luabridge::LuaRef>> mScripts;

    const AssetDB* mAssetDB = nullptr;
    ECSRegistry* mEcs = nullptr;
    InputSystem* mInputSystem = nullptr;
};

} // namespace sj