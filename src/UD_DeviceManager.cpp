#include <UD_DeviceManager.h>
#include <UD_Config.h>
#include <UD_Utility.h>
#include <UD_PapyrusDelegate.h>
#include <UD_DDAPI.h>

SINGLETONBODY(UD::DeviceManager)

void UD::DeviceManager::Reload()
{
    if (!_init || Config::GetSingleton()->GetVariable<bool>("Data.bReloadCache",false))
    {
        _init = true;
        //_jsoncache.clear();
        _DeviceTypes.clear();
        std::string loc_devconfpath = std::filesystem::current_path().string() + "\\Data\\UD\\DeviceConfig";
        std::regex loc_regex(R"regex(.*\\(.*\.[jJ][sS][oO][nN]))regex");
    
        const auto loc_vm = InternalVM::GetSingleton();
        DEBUG("Size: {}",loc_vm->objectTypeMap.size())

        std::string loc_sourcepath1 = std::filesystem::current_path().string() + "\\Data\\Scripts\\Source\\";
        std::string loc_sourcepath2 = std::filesystem::current_path().string() + "\\Data\\Source\\Scripts\\"; // SE bullshit path
        static const std::regex loc_regexDocuStr(R"regex(.*<DOCUSTR\(([\w_]+),(.*)\)>.*)regex");
        static const std::regex loc_regexExport(R"regex(.*<EXPORT\((.*)\)>.*)regex");
        static const std::regex loc_regexAtribute(R"regex(([\w_]+):[ ]*(.+))regex");
        for (auto it : loc_vm->objectTypeMap) 
        {
            if (PapyrusDelegate::GetSingleton()->HaveScriptBase(it.second.get(),"ud_customdevice_renderscript"))
            {
                DEBUG("Script {} is extending UD. Checking for docu",it.first)

                const std::string loc_sourcename = std::string(it.first) + ".psc";
                // Script extends main script. Try to import script file and search it for documentation
                std::fstream loc_sourcefile;
                loc_sourcefile.open(loc_sourcepath1 + loc_sourcename,std::ios::in);
                if (!loc_sourcefile.is_open())
                {
                    // Try other path
                    loc_sourcefile.open(loc_sourcepath2 + loc_sourcename,std::ios::in);
                }

                if (loc_sourcefile.is_open())
                {
                    DeviceConfig loc_config;

                    loc_config.script = std::string(it.first);

                    decltype(loc_config.variables) loc_vars;

                    std::string str; 
                    bool loc_luaReading = false;
                    std::string loc_luaCode = "";
                    while (std::getline(loc_sourcefile, str))
                    {
                        boost::trim(str);

                        if (str.contains("<LUA>"))
                        {
                            loc_luaReading = true;
                            continue;
                        }
                        if (str.contains("<\\LUA>"))
                        {
                            loc_luaReading = false;
                            continue;
                        }

                        if (loc_luaReading)
                        {
                            boost::replace_all(str,";","");
                            loc_luaCode += str + "\n";
                            continue;
                        }

                        if (str.contains("<DOCUSTR"))
                        {
                            const std::string loc_key   = std::regex_replace(str,loc_regexDocuStr,"$1");
                            const std::string loc_value = std::regex_replace(str,loc_regexDocuStr,"$2");
                            loc_config.docustr[loc_key] = loc_value;
                            DEBUG("Docustr found {}={} found",loc_key,loc_value)
                            continue;
                        }
                        if (str.contains("<EXPORT("))
                        {
                            DeviceVariable loc_var;
                            
                            std::vector<std::string> loc_parts;
                            boost::split(loc_parts,str,boost::is_any_of(" "),boost::algorithm::token_compress_on);
                             
                            if (loc_parts.size() >= 2)
                            {
                                std::transform(loc_parts[1].begin(),loc_parts[1].end(),loc_parts[1].begin(),::tolower);
                                if (loc_parts[1].contains("property"))
                                {
                                    loc_var.name = loc_parts[2];
                                    DEBUG("Property {} found",loc_var.name)
                                }
                                else
                                {
                                    loc_var.name = loc_parts[1];
                                    DEBUG("Variable {} found",loc_var.name)
                                }
                            }

                            loc_var.atributesRaw = std::regex_replace(str,loc_regexExport,"$1");
                            std::vector<std::string> loc_atributes;
                            boost::split(loc_atributes,loc_var.atributesRaw,boost::is_any_of(","),boost::algorithm::token_compress_on);
                            for (auto atr : loc_atributes)
                            {
                                std::string loc_atrname   = std::regex_replace(atr,loc_regexAtribute,"$1");
                                boost::trim(loc_atrname);
                                std::string loc_atrvalue  = std::regex_replace(atr,loc_regexAtribute,"$2");
                                boost::trim(loc_atrvalue);
                                loc_var.atributes[loc_atrname]  = loc_atrvalue;
                                DEBUG("{} atribute found - {}={}",loc_var.name,loc_atrname,loc_atrvalue)
                            }
                            loc_vars.push_back(loc_var);
                        }
                    }
                    

                    // Order variables by priority
                    std::sort(loc_vars.begin(),loc_vars.end(),[&](DeviceVariable& v1,DeviceVariable& v2) -> bool
                    {
                        auto loc_prio1 = boost::lexical_cast<int>(v1.atributes["prio"].value_or("0"));
                        auto loc_prio2 = boost::lexical_cast<int>(v2.atributes["prio"].value_or("0"));
                        return loc_prio1 > loc_prio2;
                    });

                    loc_config.variables = loc_vars;
                    loc_config.lua_code = loc_luaCode;
                    loc_config.parentstr = it.second->GetParent() ? it.second->GetParent()->GetName() : "";
                    loc_config.name = std::string(it.first);
                    _DeviceTypes[std::string(it.first)] = DeviceConfigPtr(new DeviceConfig(loc_config));
                    loc_sourcefile.close();
                }
                else 
                {
                    ERROR("Could not find source file for script {}",loc_sourcename)
                    continue;
                }
            }
        }
    
        InitConfigParents();
        InitConfigScripts();
    }
}

std::vector<UD::DeviceConfig> UD::DeviceManager::GetDeviceConfigs(Object a_device)
{
    return GetDeviceConfigs(a_device.get());
}

std::vector<UD::DeviceConfig> UD::DeviceManager::GetDeviceConfigs(ObjectPtr* a_device)
{
    //DEBUG("GetDeviceConfigs called")
    std::vector<std::string> loc_script;

    auto loc_info = a_device->GetTypeInfo();

    while (loc_info)
    {
        loc_script.push_back(loc_info->GetName());
        loc_info = loc_info->GetParent();
    }

    std::vector<DeviceConfig> loc_res(loc_script.size());
    for (auto it = loc_script.rbegin(); it != loc_script.rend(); ++it)
    {
        if (_DeviceTypes.find(*it) != _DeviceTypes.end())
            loc_res.push_back(*_DeviceTypes[*it]);
    }
    return loc_res;
}

float UD::DeviceManager::GetDeviceAccessibility(RE::Actor* a_actor, RE::Actor* a_helper, RE::TESObjectARMO* a_rd, bool a_checkHB)
{
    auto [loc_rd,loc_device] = PapyrusDelegate::GetSingleton()->FindDeviceScriptRD(a_actor,a_rd);
    return GetDeviceAccessibility(a_rd,loc_device.get(),a_actor,a_helper,a_checkHB);
}

float UD::DeviceManager::GetDeviceAccessibility(RE::TESObjectARMO* a_rd, ObjectPtr* a_device, RE::Actor* a_actor, RE::Actor* a_helper, bool a_checkHB)
{
    if (!a_rd || !a_device) return 0.0;

    //DEBUG("GetDeviceAccessibility called")

    float loc_res = 1.0f;
    auto loc_configs = GetDeviceConfigs(a_device);

    if (loc_configs.size() > 0)
    {
        auto loc_config = loc_configs.back();
        lua_State* L = loc_config.luaScript;
        if (L)
        {
            if (lua_getglobal(L,"GetAccessibility") != LUA_TNIL)
            {
                DeviceData loc_data;
                loc_data.config = loc_config;
                loc_data.rd = a_rd;
                if (DeviousDevicesAPI::g_API) loc_data.id = DeviousDevicesAPI::g_API->GetDeviceInventory(a_rd);
                loc_data.device = a_device;
                loc_data.wearer = a_actor;
                loc_data.helper = a_helper;

                PushDeviceData(L,loc_data);
                lua_pushboolean(L,a_checkHB);
                auto loc_luares = lua_pcall(L,2,1,0);
                if (loc_luares != LUA_OK)
                {
                    ERROR("Error running function GetAccessibility - {}",loc_luares)
                }
                else
                {
                    loc_res = lua_tonumber(L,-1);
                    lua_pop(L,1);
                }
            }
        }
        else ERROR("Cant run GetAccessibility for {}, as script is not running",loc_config.name)
    }
    else ERROR("Cant find configs for device")

    //DEBUG("GetDeviceAccessibility -> {}",loc_res)

    // TODO: Add support for hard access
    return std::clamp(loc_res,0.0f,1.0f);
}

string UD::DeviceManager::GetTags(RE::TESObjectARMO* a_rd, ObjectPtr* a_device)
{
    if (!a_rd || !a_device) return "{}";

    //DEBUG("GetTags called")
    string loc_res = "{}";

    auto loc_configs = GetDeviceConfigs(a_device);

    if (loc_configs.size() > 0)
    {
        auto loc_config = loc_configs.back();
        lua_State* L = loc_config.luaScript;
        if (L)
        {
            if (lua_getglobal(L,"GetTags") != LUA_TNIL)
            {
                DeviceData loc_data;
                loc_data.config = loc_config;
                loc_data.rd = a_rd;
                if (DeviousDevicesAPI::g_API) loc_data.id = DeviousDevicesAPI::g_API->GetDeviceInventory(a_rd);
                loc_data.device = a_device;

                PushDeviceData(L,loc_data);

                auto loc_luares = lua_pcall(L,1,1,0);
                if (loc_luares != LUA_OK)
                {
                    ERROR("Error running function GetTags - {}",loc_luares)
                }
                else
                {
                    loc_res = lua_tostring(L,-1);
                    lua_pop(L,1);
                }
            }
        }
        else ERROR("Cant run GetTags for {}, as script is not running",loc_config.name)
    }
    else ERROR("Cant find configs for device")

    return loc_res;
}

void UD::DeviceManager::PushDeviceData(lua_State* L, DeviceData& a_data)
{
    Lua::PushTable(L,
    {
        {"Wearer",a_data.wearer},
        {"Helper",a_data.helper},
        {"ID",a_data.id},
        {"RD",a_data.rd},
        {"DeviceObj",a_data.device}
    });
}

void UD::DeviceManager::InitConfigParents()
{
    for(auto&& [name,config] : _DeviceTypes)
    {
        if (config->parentstr != "" &&  _DeviceTypes.find(config->parentstr) != _DeviceTypes.end())
        {
            config->parent = _DeviceTypes[config->parentstr];
        }
    }
}

void UD::DeviceManager::InitConfigScripts()
{
    // Init scripts from parent, to make it possible to access parent function in child scripts
    for(auto&& [name,config] : _DeviceTypes)
    {
        DEBUG("Initialing scripts for {}",name)
        std::vector<DeviceConfigPtr> loc_configs;
        DeviceConfigPtr loc_config = config;
        while(loc_config)
        {
            loc_configs.push_back(loc_config);
            loc_config = loc_config->parent;
        }
        std::reverse(loc_configs.begin(), loc_configs.end());

        bool loc_open = false;
        lua_State* loc_script = nullptr;

        // Run the script from first parent to last child
        for(auto&& it : loc_configs)
        {
            if (it->lua_code != "")
            {
                //DEBUG("Loading script for {}",it->name)
                if (!loc_open) 
                {
                    loc_script = Lua::OpenScriptCode(it->lua_code);
                    loc_open = true;
                }
                else
                {
                    if (luaL_dostring(loc_script, it->lua_code.c_str()) != LUA_OK)
                    {
                        ERROR("Error opening lua code for {}",it->name)
                    }
                }
            }
        }

        config->luaScript = loc_script;
        if (config->luaScript)
        {
            DEBUG("Lua script code for DeviceConfig {} loaded",name)
        }
    }
}
