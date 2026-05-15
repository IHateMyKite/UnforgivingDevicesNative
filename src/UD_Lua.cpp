#include <UD_Lua.h>
#include <UD_PapyrusDelegate.h>
#include <UD_DeviceManager.h>
#include <UD_Utility.h>

lua_State* Lua::OpenScript(std::string a_path)
{
    lua_State* loc_res = luaL_newstate();
    luaL_openlibs(loc_res);

    if (IncludeScripts(loc_res))
    {
        RegisterHostFunctions(loc_res);
    }
    else
    {
        ERROR("Error opening including scripts")
        lua_close(loc_res);
        return nullptr;
    }

    if (luaL_dofile(loc_res, RelToAbsPath(a_path).c_str()) != LUA_OK)
    {
        ERROR("Error opening lua script {}",a_path)
        lua_close(loc_res);
        return nullptr;
    }
    return loc_res;
}

lua_State* Lua::OpenScript(std::string a_path, const std::vector<string>& a_includes)
{
    lua_State* loc_res = luaL_newstate();
    luaL_openlibs(loc_res);

    if (IncludeScripts(loc_res))
    {
        RegisterHostFunctions(loc_res);
    }
    else
    {
        ERROR("Error including global scripts")
        lua_close(loc_res);
        return nullptr;
    }

    for(auto&& it : a_includes)
    {
        if (!luaL_dofile(loc_res, RelToAbsPath(it).c_str()) == LUA_OK)
        {
            ERROR("Error include script {} to script {}",it,a_path)
            lua_close(loc_res);
            return nullptr;
        }
        else
        {
            DEBUG("Script {} included to script {}",it,a_path)
        }
    }

    if (luaL_dofile(loc_res, RelToAbsPath(a_path).c_str()) != LUA_OK)
    {
        ERROR("Error opening lua script {}",a_path)
        lua_close(loc_res);
        loc_res = nullptr;
    }

    return loc_res;
}

lua_State* Lua::OpenScriptCode(std::string a_code)
{
    lua_State* loc_res = luaL_newstate();
    luaL_openlibs(loc_res);
    if (luaL_dostring(loc_res, a_code.c_str()) == LUA_OK)
    {
        if (IncludeScripts(loc_res))
        {
            RegisterHostFunctions(loc_res);
        }
        else
        {
            ERROR("Error opening including scripts")
            lua_close(loc_res);
            loc_res = nullptr;
        }
    }
    else
    {
        ERROR("Error opening lua code \n{}",a_code)
        lua_close(loc_res);
        loc_res = nullptr;
    }
    return loc_res;
}

bool Lua::IncludeScripts(lua_State* a_script)
{
    bool loc_res = true;

    if (loc_res && (luaL_dofile(a_script, RelToAbsPath("UD\\Global.lua").c_str()) != LUA_OK))
    {
        loc_res = false;
        ERROR("Error including script {}","UD\\Global.lua")
    }

    return loc_res;
}

void Lua::RegisterHostFunctions(lua_State* L)
{
    lua_register(L,"Host_GetVariableValue",HostFunctions::lua_GetVariableValue);
    lua_register(L,"Host_UpdateVariableValue",HostFunctions::lua_UpdateVariableValue);
    lua_register(L,"Host_Log",HostFunctions::lua_Log);
    lua_register(L,"Host_ArmorHasKeyword",HostFunctions::lua_ArmorHasKeyword);
    lua_register(L,"Host_GetConfigVar",HostFunctions::lua_GetConfigVar);
    lua_register(L,"Host_CallPapyrusFunction",HostFunctions::lua_CallPapyrusFunction);
    lua_register(L,"Host_StopMinigame",HostFunctions::lua_StopMinigame);
    lua_register(L,"Host_OpenMinigameUI",HostFunctions::lua_OpenMinigameUI);
    lua_register(L,"Host_CloseMinigameUI",HostFunctions::lua_CloseMinigameUI);
    lua_register(L,"Host_ActorIsPlayer",HostFunctions::lua_ActorIsPlayer);
    lua_register(L,"Host_InvokeUI",HostFunctions::lua_InvokeUI);
    lua_register(L,"Host_IsNull",HostFunctions::lua_IsNull);
    lua_register(L,"Host_RegisterActionCallback",HostFunctions::lua_RegisterActionCallback);
    lua_register(L,"Host_GetDeviceAccesibility",HostFunctions::lua_GetDeviceAccesibility);
    lua_register(L,"Host_ActorFreeHands",HostFunctions::lua_ActorFreeHands);
    lua_register(L,"Host_WornHasKeyword",HostFunctions::lua_WornHasKeyword);
    lua_register(L,"Host_HideUI",HostFunctions::lua_HideUI);
    lua_register(L,"Host_GetGameForm",HostFunctions::lua_GetForm);
    lua_register(L,"Host_GetItemCount",HostFunctions::lua_GetItemCount);
}

bool Lua::PushTable(lua_State* L, std::vector<LuaVariable> vars)
{
    lua_createtable(L,0,vars.size());

    for (auto&& it : vars)
    {
        lua_pushstring(L,it.Name.c_str());
        switch(it.Type)
        {
            case LuaVariableType::eInteger:
                lua_pushinteger(L,LUA_CAST(it.Value,lua_Integer));
            break;
            case LuaVariableType::eNumber:
                lua_pushnumber(L,LUA_CAST(it.Value,lua_Number));
            break;
            case LuaVariableType::eBool:
                lua_pushnumber(L,it.Value[0]);
            break;
            case LuaVariableType::eString:
                lua_pushstring(L,it.Value);
            break;
            case LuaVariableType::ePtr:
            case LuaVariableType::eActor:
            case LuaVariableType::eAlias:
            case LuaVariableType::eForm:
                lua_pushlightuserdata(L,(void*)LUA_CAST(it.Value,uintptr_t));
            break;
            case LuaVariableType::eNill:
                lua_pushnil(L);
            break;
            default:
                ERROR("Invalid variable type")
                lua_pop(L,2);
                return false;
            break;
        }

        lua_settable(L,-3);
    }
    return true;
}

bool Lua::GetTable(lua_State* L,int a_indx, Lua::LuaVariable & a_res)
{
    if (!lua_istable(L,a_indx))
    {
        ERROR("No table at the given index")
        return false;
    }

    lua_pushstring(L,a_res.Name.c_str());
    lua_gettable(L,a_indx);

    if (lua_isnil(L,-1))
    {
        ERROR("Error reading value from table")
        lua_pop(L,1);
        return false;
    }

    switch(a_res.Type)
    {
        case LuaVariableType::eInteger:
            if (lua_isinteger(L,-1))
                LUA_FILL(a_res.Value,lua_tointeger(L,-1),lua_Integer);
            else ERROR("Value of {} at index {} is not integer",a_res.Name,lua_gettop(L))
        break;
        case LuaVariableType::eNumber:
            if (lua_isnumber(L,-1))
                LUA_FILL(a_res.Value,lua_tonumber(L,-1),lua_Number);
            else ERROR("Value of {} at index {} is not number",a_res.Name,lua_gettop(L))
        break;
        case LuaVariableType::eBool:
            if (lua_isboolean(L,-1))
                LUA_FILL(a_res.Value,lua_toboolean(L,-1),bool);
            else ERROR("Value of {} at index {} is not bool",a_res.Name,lua_gettop(L))
        break;
        case LuaVariableType::eString:
            if (lua_isstring(L,-1))
                strcpy_s(a_res.Value,lua_tostring(L,-1));
            else ERROR("Value of {} at index {} is not string",a_res.Name,lua_gettop(L))
        break;
        case LuaVariableType::ePtr:
        case LuaVariableType::eActor:
        case LuaVariableType::eAlias:
        case LuaVariableType::eForm:
            if (lua_isuserdata(L,-1))
                LUA_FILL(a_res.Value,lua_touserdata(L,-1),void*);
            else ERROR("Value of {} at index {} is not ptr",a_res.Name,lua_gettop(L))
        break;
        case LuaVariableType::eNill:
        break;
        default:
            ERROR("Invalid variable type")
            lua_pop(L,1);
            return false;
        break;
    }

    lua_pop(L,1);
    return true;
}

bool Lua::ParseArgTable(lua_State* L, int a_indx, std::vector<LuaVariablePtr>& a_res)
{
    if (!lua_istable(L,a_indx))
    {
        ERROR("No arg table at the given index")
        return false;
    }

    //DEBUG("ParseArgTable called")

    Lua::LuaVariable loc_n("n",(lua_Integer)0);
    if (!GetTable(L,a_indx, loc_n))
    {
        ERROR("Error reading size of arg table")
        lua_pop(L,1);
        return false;
    }

    //DEBUG("ParseArgTable - Number of args = {}",*(lua_Integer*)&loc_n.Value)

    for (lua_Integer i = 0; i < *(lua_Integer*)&loc_n.Value; i++)
    {
        Lua::LuaVariable loc_type(std::to_string(i) + "_t",std::string());
        if (!GetTable(L,a_indx, loc_type))
        {
            ERROR("Error reading value from arg table")
            lua_pop(L,1);
            return false;
        }

        //DEBUG("ParseArgTable - Type is {}",std::string(loc_type.Value))

        Lua::LuaVariablePtr loc_value = Lua::LuaVariablePtr(new Lua::LuaVariable(std::to_string(i) + "_v"));
        loc_value->Type = ParseArgTypeStr(std::string(loc_type.Value));

        if (!GetTable(L,a_indx, *loc_value))
        {
            ERROR("Error reading value from arg table")
            lua_pop(L,1);
            return false;
        }

        a_res.push_back(loc_value);
    }

    lua_pop(L,1);
    return true;
}

void Lua::PushVariableResult(lua_State* L, UD::VariableValue& a_val)
{
    //DEBUG("PushVariableResult({},{}) called",(int)a_val.Type,a_val.Value)
    switch(a_val.Type)
    {
        case VariableType::kInt:
        {
            lua_pushinteger(L,UD::GetValue<int>(a_val));
        }
        break;
        case VariableType::kBool:
        {
            lua_pushboolean(L,UD::GetValue<bool>(a_val));
        }
        break;
        case VariableType::kFloat:
        {
            lua_pushnumber(L,UD::GetValue<float>(a_val));
        }
        break;
        case VariableType::kString:
        {
            lua_pushstring(L,a_val.Value.c_str());
        }
        break;
        case VariableType::kIntArray:
        {
            std::vector<int> loc_values = UD::Utility::ConvertStringToArray<int>(a_val.Value);
            lua_createtable(L,loc_values.size(),1);
            lua_pushstring(L,"n");
            lua_pushinteger(L,loc_values.size());
            lua_settable(L,-3);
            for(int i = 0; i < loc_values.size(); i++)
            {
                lua_pushinteger(L,loc_values[i]);
                lua_seti(L,-2,i);
            }
        }
        break;
        case VariableType::kBoolArray:
        {
            std::vector<bool> loc_values = UD::Utility::ConvertStringToArray<bool>(a_val.Value);
            lua_createtable(L,loc_values.size(),1);
            lua_pushstring(L,"n");
            lua_pushinteger(L,loc_values.size());
            lua_settable(L,-3);
            for(int i = 0; i < loc_values.size(); i++)
            {
                lua_pushboolean(L,loc_values[i]);
                lua_seti(L,-2,i);
            }
        }
        break;
        case VariableType::kFloatArray:
        {
            std::vector<float> loc_values = UD::Utility::ConvertStringToArray<float>(a_val.Value);
            lua_createtable(L,loc_values.size(),1);
            lua_pushstring(L,"n");
            lua_pushinteger(L,loc_values.size());
            lua_settable(L,-3);
            for(int i = 0; i < loc_values.size(); i++)
            {
                lua_pushnumber(L,loc_values[i]);
                lua_seti(L,-2,i);
            }
        }
        break;
        case VariableType::kStringArray:
        {
            std::vector<string> loc_values = UD::Utility::ConvertStringToArray<string>(a_val.Value);
            lua_createtable(L,loc_values.size(),1);
            lua_pushstring(L,"n");
            lua_pushinteger(L,loc_values.size());
            lua_settable(L,-3);

            // Set value
            for(int i = 0; i < loc_values.size(); i++)
            {
                lua_pushstring(L,loc_values[i].c_str());
                lua_seti(L,-2,i);
            }
        }
        break;
        default:
            lua_pushnil(L);
            ERROR("PushVariableResult - Unsupported type {}",(int)a_val.Type)
        break;
    }
}

Lua::LuaVariableType Lua::ParseArgTypeStr(std::string a_var)
{
    Lua::LuaVariableType loc_res;
    if      (a_var == "int")    loc_res = Lua::LuaVariableType::eInteger;
    else if (a_var == "float")  loc_res = Lua::LuaVariableType::eNumber;
    else if (a_var == "bool")   loc_res = Lua::LuaVariableType::eBool;
    else if (a_var == "string") loc_res = Lua::LuaVariableType::eString;
    else if (a_var == "form")   loc_res = Lua::LuaVariableType::eForm;
    else if (a_var == "actor")  loc_res = Lua::LuaVariableType::eActor;
    else if (a_var == "alias")  loc_res = Lua::LuaVariableType::eAlias;
    else loc_res = Lua::LuaVariableType::eNill;
    return loc_res;
}

int Lua::HostFunctions::lua_GetVariableValue(lua_State* L)
{
    if (!lua_istable(L,1) || !lua_isstring(L,2))
    {
        ERROR("lua_GetVariableValue - Incorrect variables passed!")
        lua_pushnil(L);
        return 1;
    }

    std::string         loc_var     = lua_tostring(L,2);
    UD::VariableDetails loc_det     = UD::ParseVariable(loc_var);

    LuaVariable loc_wearerVar("Wearer",(void*)nullptr);
    GetTable(L,1,loc_wearerVar);
    LuaVariable loc_helperVar("Helper",(void*)nullptr);
    GetTable(L,1,loc_helperVar);
    LuaVariable loc_deviceVar("DeviceObj",(void*)nullptr);
    GetTable(L,1,loc_deviceVar);
    RE::Actor*  loc_wearer      = *(RE::Actor**)&loc_wearerVar.Value;
    RE::Actor*  loc_helper      = *(RE::Actor**)&loc_helperVar.Value;
    ObjectPtr*  loc_device     = *(ObjectPtr**)&loc_deviceVar.Value;

    void* loc_owner = nullptr;
    if (loc_det.Owner == "thisdevice" && loc_device)
    {
        loc_owner = loc_device;
    }
    else if (loc_det.Owner == "wearer" && loc_wearer)
    {
        loc_owner = loc_wearer;
    }
    else if (loc_det.Owner == "helper" && loc_helper)
    {
        loc_owner = loc_helper;
    }

    if (loc_owner)
    {
        auto loc_val = UD::GetVariableRaw(loc_owner,loc_det);
        PushVariableResult(L,loc_val);
    }
    else
    {
        lua_pushnil(L);
    }
    return 1;
}

int Lua::HostFunctions::lua_UpdateVariableValue(lua_State* L)
{
    if (!lua_istable(L,1) || !lua_isstring(L,2) || lua_isnil(L,3))
    {
        ERROR("lua_SetVariableValue - Incorrect variables passed!")
        lua_pushnil(L);
        return 1;
    }

    std::string         loc_var     = lua_tostring(L,2);
    UD::VariableDetails loc_det     = UD::ParseVariable(loc_var);

    LuaVariable loc_wearerVar("Wearer",(void*)nullptr);
    GetTable(L,1,loc_wearerVar);
    LuaVariable loc_helperVar("Helper",(void*)nullptr);
    GetTable(L,1,loc_helperVar);
    LuaVariable loc_deviceVar("DeviceObj",(void*)nullptr);
    GetTable(L,1,loc_deviceVar);
    RE::Actor*  loc_wearer      = *(RE::Actor**)&loc_wearerVar.Value;
    RE::Actor*  loc_helper      = *(RE::Actor**)&loc_helperVar.Value;
    ObjectPtr*   loc_device     = *(ObjectPtr**)&loc_deviceVar.Value;

    void* loc_owner = nullptr;
    if (loc_det.Owner == "thisdevice" && loc_device)
    {
        loc_owner = loc_device;
    }
    else if (loc_det.Owner == "wearer" && loc_wearer)
    {
        loc_owner = loc_wearer;
    }
    else if (loc_det.Owner == "helper" && loc_helper)
    {
        loc_owner = loc_helper;
    }

    UD::VariableValue loc_val;
    // Parse new value
    switch (lua_type(L,3))
    {
        case LUA_TNUMBER:
        {
            auto loc_number = lua_tonumber(L,3);
            if (trunc(loc_number) == loc_number)
            {
                loc_val.Value = std::to_string((int)loc_number);
                loc_val.Type = VariableType::kInt;
            }
            else
            {
                loc_val.Value = std::to_string((float)loc_number);
                loc_val.Type = VariableType::kFloat;
            }
        }
        break;
        case LUA_TBOOLEAN:
            loc_val.Value = std::to_string(lua_toboolean(L,3));
            loc_val.Type = VariableType::kBool;
        break;
        case LUA_TSTRING:
            loc_val.Value = std::string(lua_tostring(L,3));
            loc_val.Type = VariableType::kString;
        break;
        default:
            ERROR("Unsupported value type")
            lua_pushnil(L);
            return 1;
        break;
    }

    if (loc_owner)
    {
        auto loc_res = UD::SetVariableRaw(loc_owner,loc_det,loc_val);
        PushVariableResult(L,loc_res);
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

int Lua::HostFunctions::lua_ArmorHasKeyword(lua_State* L)
{
    if (!lua_isuserdata(L,1) || !lua_isstring(L,2))
    {
        ERROR("lua_DeviceHasKeyword - Incorrect variables passed!")
        lua_pushnil(L);
        return 1;
    }
    
    RE::TESObjectARMO* loc_armor = (RE::TESObjectARMO*)lua_touserdata(L,1);
    std::string loc_keyword = lua_tostring(L,2);
    if (loc_armor)
    {
        const bool loc_res = loc_armor->HasKeywordString(loc_keyword);
        lua_pushboolean(L,loc_res);
    }
    else lua_pushnil(L);
    
    return 1;
}

int Lua::HostFunctions::lua_GetConfigVar(lua_State* L)
{
    if (!lua_istable(L,1) || !lua_isstring(L,2) || !lua_isstring(L,3))
    {
        ERROR("lua_GetConfigVar - Incorrect variables passed!")
        lua_pushnil(L);
        return 1;
    }

    LuaVariable loc_jsonVar("Json",(void*)nullptr);
    GetTable(L,1,loc_jsonVar);

    std::string loc_var     = lua_tostring(L,2);
    std::string loc_def     = lua_tostring(L,3);

    auto loc_json = *(boost::property_tree::ptree**)&loc_jsonVar.Value;
    auto loc_config = loc_json->get_child_optional("config");
    if (loc_config.has_value())
    {
        std::string loc_res = loc_config.get().get_optional<std::string>(loc_var).get_value_or(loc_def);
        lua_pushstring(L,loc_res.c_str());
        return 1;
    }

    lua_pushnil(L);
    return 1;
}

int Lua::HostFunctions::lua_CallPapyrusFunction(lua_State* L)
{
    if (!lua_istable(L,1) || !lua_isstring(L,2) || !lua_isstring(L,3) || (!lua_istable(L,4) && !lua_isnil(L,4)))
    {
        ERROR("lua_CallPapyrusFunction - Incorrect variables passed!")
        return 0;
    }

    //DEBUG("lua_CallPapyrusFunction called")

    std::string         loc_var     = lua_tostring(L,2);
    UD::FunctionDetails loc_det     = UD::ParseFunction(loc_var);
    UD::PapFunc loc_func;
    loc_func.Callback = loc_det.Name;
    loc_func.Module   = loc_det.Owner;
    
    if (loc_det.Owner == "thisdevice")
    {
        LuaVariable loc_deviceVar("DeviceObj",(void*)nullptr);
        GetTable(L,1,loc_deviceVar);
        ObjectPtr*  loc_device  = *(ObjectPtr**)&loc_deviceVar.Value;
        loc_func.Source   = Object(loc_device);
    }
    else
    {
        auto loc_module = UD::ModuleManager::GetSingleton()->GetModuleObjectByAlias(loc_det.Owner);
        loc_func.Source   = loc_module->object;
    }

    std::vector<LuaVariablePtr> loc_args;

    LuaVariable loc_indexVar("MinigameId",(lua_Integer)0);
    GetTable(L,1,loc_indexVar);
    loc_func.Id         = *(int*)&loc_indexVar.Value;
    loc_func.Callback2  = lua_tostring(L,3);
    
    if (!lua_isnil(L,4) && ParseArgTable(L,4,loc_args))
    {
        std::vector<UD::FuncArgPtr> loc_funcArgs;
        for(auto&& it : loc_args)
        {
            switch(it->Type)
            {
                case LuaVariableType::eInteger:
                    loc_funcArgs.push_back(UD::FuncArgPtr(new UD::FuncArg((*(int*)&it->Value))));
                break;
                case LuaVariableType::eNumber:
                    loc_funcArgs.push_back(UD::FuncArgPtr(new UD::FuncArg((*(float*)&it->Value))));
                break;
                case LuaVariableType::eBool:
                    loc_funcArgs.push_back(UD::FuncArgPtr(new UD::FuncArg((*(bool*)&it->Value))));
                break;
                case LuaVariableType::eString:
                    loc_funcArgs.push_back(UD::FuncArgPtr(new UD::FuncArg(std::string(it->Value))));
                break;
                case LuaVariableType::eActor:
                    loc_funcArgs.push_back(UD::FuncArgPtr(new UD::FuncArg((*(RE::Actor**)&it->Value))));
                break;
                case LuaVariableType::eForm:
                    loc_funcArgs.push_back(UD::FuncArgPtr(new UD::FuncArg((*(RE::TESForm**)&it->Value))));
                break;
                case LuaVariableType::eAlias:
                    loc_funcArgs.push_back(UD::FuncArgPtr(new UD::FuncArg((*(RE::BGSBaseAlias**)&it->Value))));
                break;
            }
        }
        loc_func.Args = loc_funcArgs;
    }
    
    UD::CallPapyrusFunc(loc_func);

    return 0;
}

int Lua::HostFunctions::lua_StopMinigame(lua_State* L)
{
    if (!lua_isinteger(L,1))
    {
        ERROR("lua_StopMinigame - Incorrect variables passed!")
        return 0;
    }

    DEBUG("lua_StopMinigame({})",lua_tointeger(L,1))
    UD::MinigameManager::GetSingleton()->StopMinigame(lua_tointeger(L,1));
    return 0;
}

int Lua::HostFunctions::lua_OpenMinigameUI(lua_State* L)
{
    if (!lua_isinteger(L,1) || !lua_isstring(L,2))
    {
        ERROR("lua_OpenMinigameUI - Incorrect variables passed!")
        return 0;
    }
    DEBUG("lua_OpenMinigameUI({},{})",lua_tointeger(L,1),lua_tostring(L,2))
    UD::MinigameManager::GetSingleton()->OpenMinigameUI(lua_tointeger(L,1),lua_tostring(L,2));
    return 0;
}

int Lua::HostFunctions::lua_CloseMinigameUI(lua_State* L)
{
    if (!lua_isinteger(L,1))
    {
        ERROR("lua_CloseMinigameUI - Incorrect variables passed!")
        return 0;
    }
    DEBUG("lua_CloseMinigameUI({})",lua_tointeger(L,1))
    UD::MinigameManager::GetSingleton()->CloseMinigameUI(lua_tointeger(L,1));
    return 0;
}

int Lua::HostFunctions::lua_ActorIsPlayer(lua_State* L)
{
    if (!lua_isuserdata(L,1))
    {
        ERROR("lua_ActorIsPlayer - Incorrect variables passed!")
        lua_pushboolean(L,false);
        return 1;
    }
    
    RE::Actor* loc_actor = (RE::Actor*)lua_touserdata(L,1);
    if (loc_actor)
    {
        const bool loc_res = loc_actor->IsPlayerRef();
        lua_pushboolean(L,loc_res);
    }
    else lua_pushboolean(L,false);
    
    return 1;
}

int Lua::HostFunctions::lua_InvokeUI(lua_State* L)
{
    if (!lua_isinteger(L,1) || !lua_isstring(L,2))
    {
        ERROR("lua_InvokeUI - Incorrect variables passed!")
        return 0;
    }

    //DEBUG("lua_InvokeUI({})",lua_tostring(L,2))

    UD::MinigameManager::GetSingleton()->InvokeUI(lua_tostring(L,2));

    return 0;
}

int Lua::HostFunctions::lua_IsNull(lua_State* L)
{
    if (!lua_isuserdata(L,1))
    {
        ERROR("lua_IsNull - Incorrect variables passed!")
        lua_pushboolean(L,true);
        return 1;
    }
    void* loc_ptr = lua_touserdata(L,1);
    lua_pushboolean(L,loc_ptr == NULL);
    return 1;
}

int Lua::HostFunctions::lua_RegisterActionCallback(lua_State* L)
{
    if (!lua_isinteger(L,1) || !lua_isstring(L,2) || !lua_isstring(L,3))
    {
        ERROR("lua_RegisterActionCallback - Incorrect variables passed!")
        return 0;
    }

    auto loc_data = UD::MinigameManager::GetSingleton()->GetMinigameDataById(lua_tointeger(L,1));
    if (loc_data)
    {
        auto loc_control = UD::ControlManager::GetSingleton()->GetActionControl(lua_tostring(L,2));
        UD::MinigameActionCallback loc_callback;
        loc_callback.control    = loc_control;
        loc_callback.callback   = lua_tostring(L,3);
        DEBUG("Action callback registered - {} , {} , {}",loc_callback.callback,loc_callback.control.alias,loc_callback.control.codekeyboard)

        auto loc_find = std::find_if(loc_data->Controls.begin(),loc_data->Controls.end(),[loc_control](UD::MinigameActionCallback& a_callback)
        {
            return a_callback.control == loc_control;
        });

        if (loc_find != loc_data->Controls.end())
        {
            *loc_find._Ptr = loc_callback;
        }
        else loc_data->Controls.push_back(loc_callback);
    }

    return 0;
}

int Lua::HostFunctions::lua_GetDeviceAccesibility(lua_State* L)
{
    if (!lua_istable(L,1))
    {
        ERROR("lua_GetDeviceAccesibility - Incorrect variables passed!")
        lua_pushnil(L);
        return 1;
    }

    LuaVariable loc_wearerVar("Wearer",(void*)nullptr);
    GetTable(L,1,loc_wearerVar);
    LuaVariable loc_helperVar("Helper",(void*)nullptr);
    GetTable(L,1,loc_helperVar);
    LuaVariable loc_deviceVar("DeviceObj",(void*)nullptr);
    GetTable(L,1,loc_deviceVar);
    LuaVariable loc_rdVar("RD",(void*)nullptr);
    GetTable(L,1,loc_rdVar);
    LuaVariable loc_idVar("ID",(void*)nullptr);
    GetTable(L,1,loc_idVar);

    RE::Actor*  loc_wearer      = *(RE::Actor**)&loc_wearerVar.Value;
    RE::Actor*  loc_helper      = *(RE::Actor**)&loc_helperVar.Value;
    ObjectPtr*   loc_device     = *(ObjectPtr**)&loc_deviceVar.Value;
    RE::TESObjectARMO*  loc_rd  = *(RE::TESObjectARMO**)&loc_rdVar.Value;

    const float loc_Res = UD::DeviceManager::GetSingleton()->GetDeviceAccessibility(loc_rd,loc_device,loc_wearer,loc_helper);
    lua_pushnumber(L,loc_Res);

    return 1;
}

int Lua::HostFunctions::lua_ActorFreeHands(lua_State* L)
{
    if (!lua_isuserdata(L,1) || !lua_isboolean(L,2) || !lua_isboolean(L,3))
    {
        ERROR("lua_ActorFreeHands - Incorrect variables passed!")
        lua_pushnil(L);
        return 1;
    }

    RE::Actor* loc_actor        = (RE::Actor*)lua_touserdata(L,1);
    const bool loc_checkgrasp   = lua_toboolean(L,2);
    const bool loc_ignorehb     = lua_toboolean(L,3);

    const bool loc_res = UD::Utility::ActorFreeHands(loc_actor,loc_checkgrasp,loc_ignorehb);

    lua_pushboolean(L,loc_res);
    return 1;
}

int Lua::HostFunctions::lua_WornHasKeyword(lua_State* L)
{
    if (!lua_isuserdata(L,1) || !lua_isstring(L,2))
    {
        ERROR("lua_WornHasKeyword - Incorrect variables passed!")
        lua_pushboolean(L,false);
        return 1;
    }

    RE::Actor*  loc_actor   = (RE::Actor*)lua_touserdata(L,1);
    std::string loc_kw      = lua_tostring(L,2);
    const bool  loc_res     = UD::Utility::WornHasKeyword(loc_actor,loc_kw);

    lua_pushboolean(L,loc_res);

    return 1;
}

int Lua::HostFunctions::lua_HideUI(lua_State* L)
{
    if (!lua_isinteger(L,1))
    {
        ERROR("lua_HideUI - Incorrect variables passed!")
        return 0;
    }

    auto loc_data = UD::MinigameManager::GetSingleton()->GetMinigameDataById(lua_tointeger(L,1));
    if (loc_data)
    {
        
        
    }

    return 0;
}

int Lua::HostFunctions::lua_GetForm(lua_State* L)
{
    if (!lua_isinteger(L,1) || !lua_isstring(L,2))
    {
        ERROR("lua_GetForm - Incorrect variables passed!")
        lua_pushnil(L);
        return 1;
    }
    
    const int loc_formid = lua_tointeger(L,1);
    const string loc_mod = lua_tostring(L,2);

    RE::TESForm* loc_res = RE::TESDataHandler::GetSingleton()->LookupForm(loc_formid,loc_mod);

    if (loc_res)
    {
        lua_pushlightuserdata(L,loc_res);
        return 1;
    }
    else
    {
        lua_pushnil(L);
        return 1;
    }
}

int Lua::HostFunctions::lua_GetItemCount(lua_State* L)
{
    if (!lua_islightuserdata(L,1) || (!lua_islightuserdata(L,2) && !lua_isstring(L,2)))
    {
        ERROR("lua_GetItemCount - Incorrect variables passed!")
        lua_pushinteger(L,0);
        return 1;
    }

    auto loc_container = (RE::Actor*)lua_touserdata(L,1);

    RE::TESObjectREFR::InventoryCountMap loc_Counts;

    if (lua_islightuserdata(L,2))
    {
        auto loc_item = (RE::TESForm*)lua_touserdata(L,2);

        loc_Counts = loc_container->GetInventoryCounts([loc_item](RE::TESBoundObject& a_item)
        {
            if (loc_item && a_item.formID == loc_item->formID)
            {
                return true;
            }
            return false;
        });
    }
    else if (lua_isstring(L,2))
    {
        string loc_itemStr = lua_tostring(L,2);

        loc_Counts = loc_container->GetInventoryCounts([loc_itemStr](RE::TESBoundObject& a_item)
        {
            if (a_item.GetFormEditorID() == loc_itemStr)
            {
                return true;
            }

            return false;
        });
    }

    int loc_res = 0;
    for(auto it : loc_Counts)
    {
        loc_res += it.second;
    }

    lua_pushinteger(L,loc_res);
    return 1;
}
