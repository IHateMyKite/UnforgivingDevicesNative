#pragma once
#include <lua.hpp>
#include <UD_VariableManager.h>

namespace Lua
{
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
        RE::Actor* Wearer;
        RE::Actor* Helper;
        Object DeviceObj;
    };

    lua_State* OpenScript(std::string a_path);

    void RegisterHostFunctions(lua_State* L);

    namespace HostFunctions
    {
         int lua_GetVariableValue(lua_State* L);
         int lua_Log(lua_State* L);
    
    }



}