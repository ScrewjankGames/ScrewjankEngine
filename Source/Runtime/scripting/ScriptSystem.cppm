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

export module sj.engine.scripting:ScriptSystem;
import :Signal;
import :LuauStack;
import :TypeRegistration;

import sj.std;
import sj.datadefs;
import sj.engine.ecs;
import sj.engine.system.threading.ThreadContext;
import sj.engine.InputSystem;

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

        [[maybe_unused]] luabridge::Result res = luabridge::push(L.get(), GameObject(goId, *mEcs));
        scriptIt->second->call(luabridge::LuaRef::fromStack(L.get()));
    }

    void OnDestroy(GameObjectId goId, ScriptComponent* component)
    {
        auto scriptIt = mScripts.find(component->script_id);
        scriptIt->second.refcount_decrement();
    }

private:

    static luabridge::LuaRef LoadScript(lua_State* L, std::string_view scriptPath)
    {
        std::ifstream file(scriptPath.data(), std::ios::binary | std::ios::ate);
        SJ_ASSERT(file.is_open(), "Unable to load script component- file not found");
        std::streamsize size = file.tellg();
        file.seekg(0, std::ios::beg);

        auto scope = ThreadContext::GetScratchpad();
        std::pmr::vector<char> bytecode(&scope);
        bytecode.reserve(size);
        file.read(bytecode.data(), size);

        std::pmr::string chunkName(&scope);
        chunkName = scriptPath;

        luau_load(L, chunkName.c_str(), bytecode.data(), bytecode.size(), 0);
        luabridge::LuaRef res = luabridge::LuaRef::fromStack(L, -1);
        return res;
    }

    void SetupEnv(lua_State* L)
    {
        RegisterTypes(L);
        
        // Additional Types
        luabridge::getGlobalNamespace(L)
            .beginClass<ProcessSignal>("ProcessSignal")
            .addFunction("Connect", &ProcessSignal::Connect)
            .endClass();

        // Globals
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