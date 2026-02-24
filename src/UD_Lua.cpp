#include <UD_Lua.h>
#include <UD_PapyrusDelegate.h>

lua_State* Lua::OpenScript(std::string a_path)
{
    lua_State* loc_res = luaL_newstate();
    luaL_openlibs(loc_res);
    if (luaL_dofile(loc_res, RelToAbsPath(a_path).c_str()) == LUA_OK)
    {
        RegisterHostFunctions(loc_res);
    }
    else
    {
        ERROR("Error opening lua script {}",a_path)
        lua_close(loc_res);
        loc_res = nullptr;
    }
    return loc_res;
}

void Lua::RegisterHostFunctions(lua_State* L)
{
    lua_register(L,"GetVariableValue",HostFunctions::lua_GetVariableValue);
    lua_register(L,"Log",HostFunctions::lua_Log);
}

int Lua::HostFunctions::lua_GetVariableValue(lua_State* L)
{
    if (!lua_isuserdata(L,1) || !lua_isstring(L,2))
    {
        ERROR("lua_GetVariableValue - Incorrect variables passed!")
        lua_pushnil(L);
        return 1;
    }

    LuaContext*         loc_context = (LuaContext*)lua_touserdata(L,1);
    std::string         loc_var     = lua_tostring(L,2);
    UD::VariableDetails loc_det     = UD::ParseVariable(loc_var);

    //DEBUG("lua_GetVariableValue({}) called",loc_var)

    RE::Actor*  loc_wearer      = loc_context->Wearer;
    Object      loc_deviceObj   = loc_context->DeviceObj;

    void* loc_owner = nullptr;
    if (loc_det.Owner == "thisdevice" && loc_deviceObj)
    {
        loc_owner = loc_deviceObj.get();
    }
    else if (loc_det.Owner == "wearer" && loc_wearer)
    {
        loc_owner = loc_wearer;
    }

    if (loc_owner)
    {
        auto loc_val = UD::GetVariableRaw(loc_owner,loc_det);
        switch(loc_val.Type)
        {
            case VariableType::kInt:
            {
                lua_pushinteger(L,UD::GetValue<int>(loc_val));
            }
            break;
            case VariableType::kBool:
            {
                lua_pushboolean(L,UD::GetValue<bool>(loc_val));
            }
            break;
            case VariableType::kFloat:
            {
                lua_pushnumber(L,UD::GetValue<float>(loc_val));
            }
            break;
            case VariableType::kString:
            {
                lua_pushstring(L,loc_val.Value.c_str());
            }
            break;
            default:
                lua_pushnil(L);
                ERROR("lua_GetVariableValue - Unsupported type")
            break;
        }
    }
    else
    {
        lua_pushnil(L);
    }
    return 1;
}

int Lua::HostFunctions::lua_Log(lua_State* L)
{
    std::string loc_arg = lua_tostring(L,1);
    DEBUG("L[ {} ]",loc_arg)
    return 0;
}
