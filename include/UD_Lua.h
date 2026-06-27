#pragma once
#include <lua.hpp>
#include <UD_VariableManager.h>
#include <UD_CallbackManager.h>
#include <UD_ModuleManager.h>
#include <UD_MinigameManager.h>

namespace Lua
{
    #define LUA_CAST(x,y) *reinterpret_cast<const y * const>(&x)
    #define LUA_FILL(x,y,z) *reinterpret_cast<z *>(&x) = y
    #define LUA_CHECK(x,y) \
    {                      \
        if (x != LUA_OK)   \
        {                  \
            ERROR(y)       \
            return;        \
        }                  \
    }
    #define LUA_CHECKNIL(x,y) \
    {                      \
        if (x == LUA_TNIL) \
        {                  \
            ERROR(y)       \
            return;        \
        }                  \
    }

    struct InitValue
    {
        RE::Actor* Wearer;
        RE::Actor* Helper;
        RE::TESObjectARMO* ID;
    };

    struct LuaContext
    {
        void* Contexts[20U];
    };

    enum class LuaVariableType
    {
        eNill,
        eInteger,
        eNumber,
        eString,
        eBool,
        ePtr,
        eActor,
        eForm,
        eAlias
    };

    struct LuaVariable
    {
        LuaVariable(std::string a_name, lua_Number a_var) : Name(a_name), Type(LuaVariableType::eNumber) { *(lua_Number*)&Value = a_var;}
        LuaVariable(std::string a_name, lua_Integer a_var) : Name(a_name), Type(LuaVariableType::eInteger) { *(lua_Integer*)&Value = a_var;}
        LuaVariable(std::string a_name, bool a_var) : Name(a_name), Type(LuaVariableType::eBool) { *(bool*)&Value = a_var;}
        LuaVariable(std::string a_name, std::string a_var) : Name(a_name), Type(LuaVariableType::eString) { strcpy_s(Value,a_var.c_str());}
        LuaVariable(std::string a_name, void* a_var) : Name(a_name), Type(LuaVariableType::ePtr) { *(void**)&Value = a_var;}
        LuaVariable(std::string a_name, RE::Actor* a_var) : Name(a_name), Type(LuaVariableType::eActor) { *(RE::Actor**)&Value = a_var;}
        LuaVariable(std::string a_name, RE::TESForm* a_var) : Name(a_name), Type(LuaVariableType::eActor) { *(RE::TESForm**)&Value = a_var;}
        LuaVariable(std::string a_name, RE::BGSBaseAlias* a_var) : Name(a_name), Type(LuaVariableType::eActor) { *(RE::BGSBaseAlias**)&Value = a_var;}
        LuaVariable(std::string a_name) : Name(a_name), Type(LuaVariableType::eNill) {}
        std::string Name;
        char Value[64U];
        LuaVariableType Type;
    };

    typedef std::shared_ptr<LuaVariable> LuaVariablePtr;

    lua_State* OpenScript(std::string a_path);
    lua_State* OpenScript(std::string a_path,const std::vector<string>& a_includes);
    lua_State* OpenScriptCode(std::string a_code);

    bool IncludeScripts(lua_State* a_script);

    void RegisterHostFunctions(lua_State* L);

    bool PushTable(lua_State* L,std::vector<LuaVariable> vars);
    bool GetTable(lua_State* L,int a_indx, LuaVariable & a_res);
    bool ParseArgTable(lua_State* L,int a_indx, std::vector<LuaVariablePtr> & a_res);
    void PushVariableResult(lua_State* L,UD::VariableValue& a_val);
    LuaVariableType ParseArgTypeStr(std::string a_var);

    namespace HostFunctions
    {
        int lua_GetVariableValue(lua_State* L);
        int lua_UpdateVariableValue(lua_State* L);
        int lua_Log(lua_State* L);
        int lua_ArmorHasKeyword(lua_State* L);
        int lua_GetConfigVar(lua_State* L);
        int lua_CallPapyrusFunction(lua_State* L);
        int lua_StopMinigame(lua_State* L);
        int lua_OpenMinigameUI(lua_State* L);
        int lua_CloseMinigameUI(lua_State* L);
        int lua_ActorIsPlayer(lua_State* L);
        int lua_InvokeUI(lua_State* L);
        int lua_IsNull(lua_State* L);
        int lua_RegisterActionCallback(lua_State* L);
        int lua_GetDeviceAccesibility(lua_State* L);
        int lua_ActorFreeHands(lua_State* L);
        int lua_WornHasKeyword(lua_State* L);
        int lua_HideUI(lua_State* L);
        int lua_GetForm(lua_State* L);
        int lua_GetItemCount(lua_State* L);
        int lua_GetHudValue(lua_State* L);
    }

}