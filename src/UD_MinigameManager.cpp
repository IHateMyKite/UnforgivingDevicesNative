#include <UD_MinigameManager.h>
#include <UD_Utility.h>
#include <UD_Config.h>
#include <UD_ModuleManager.h>
#include <UD_PapyrusDelegate.h>

SINGLETONBODY(UD::MinigameManager)

PRISMA_UI_API::IVPrismaUI1* UD::MinigameManager::PrismaUI = nullptr;

void UD::MinigameManager::Reload()
{
    auto loc_apiptr = PRISMA_UI_API::RequestPluginAPI();
    PrismaUI = reinterpret_cast<PRISMA_UI_API::IVPrismaUI1*>(loc_apiptr);

    if (!_init || Config::GetSingleton()->GetVariable<bool>("Data.bReloadCache",false))
    {
        _init = true;
        _jsoncache.clear();

        std::string loc_devconfpath = RelToAbsPath("UD\\MinigameConfig");
        std::regex loc_regex(R"regex(.*\\(.*\.[jJ][sS][oO][nN]))regex");
        uint32_t loc_id = 0;
        for (const auto & entry : std::filesystem::directory_iterator(loc_devconfpath))
        {
            std::string loc_path = entry.path().string();
    
            if (entry.is_regular_file() && std::regex_match(loc_path,loc_regex)) 
            {
                const std::string loc_jsonname = std::regex_replace(loc_path,loc_regex,"$1");
                std::shared_ptr<boost::property_tree::ptree> loc_json = std::shared_ptr<boost::property_tree::ptree>(new boost::property_tree::ptree);
                try
                {
                    boost::property_tree::read_json(loc_path, *loc_json.get());
                }
                catch(const std::exception& e)
                {
                    ERROR("Error parsing json {} - {}",loc_jsonname,e.what())
                    continue;
                }
    
                std::regex loc_regexname(R"regex((.*\\)(.*)(\.[jJ][sS][oO][nN]))regex");
                const std::string loc_name = std::regex_replace(loc_path,loc_regexname,"$2");
    
                auto loc_config = std::shared_ptr<MinigameConfigJson>(new MinigameConfigJson{loc_id++,loc_json,MinigameConfigStatus::sOK,"OK"});
                InitConfig(loc_config);
                
                _jsoncache[loc_name] = loc_config;
            }
        }
    
        DEBUG("=== Loaded device config files ===")
        for (auto&& [name,file] : _jsoncache)
        {
            DEBUG("\t{} - {} / {}",name, file->status, file->error)
        }
    }
}

std::vector<std::string> UD::MinigameManager::GetListOfMinigamesStr(RE::Actor* a_actor, RE::TESObjectARMO* a_id)
{
    DEBUG("GetListOfMinigamesStr called")
    std::vector<std::string> loc_res;
    auto loc_minigames = GetListOfMinigames(a_actor,nullptr,a_id);
    for(auto&& it : loc_minigames) loc_res.push_back(it->config.name);
    return loc_res;
}

std::vector<UD::MinigameSetting> UD::MinigameManager::GetListOfMinigames(RE::Actor* a_actor, RE::Actor* a_helper, RE::TESObjectARMO* a_id)
{
    DEBUG("GetListOfMinigames called")
    std::vector<std::shared_ptr<UD::MinigameConfigJson>> loc_res;

    DeviceObj loc_device = PapyrusDelegate::GetSingleton()->FindDeviceScriptID(a_actor,a_id);

    if (a_actor && loc_device.second)
    {
        for (auto&& [path,config] : _jsoncache)
        {
            auto L = _scripts[config->config.script];
            if (!L) continue;

            lua_getglobal(L,"Precondition");
            DEBUG("loc_device = 0x{:016X}",(uintptr_t)&loc_device)
            Lua::PushTable(L,
            {
                {"Wearer",a_actor},
                {"Helper",a_helper},
                {"ID",a_id},
                {"RD",loc_device.first},
                {"DeviceObj",loc_device.second.get()},
                {"Json",config->json.get()}
            });

            lua_pcall(L,1,1,0);
            const auto loc_precond = lua_toboolean(L,-1);
            lua_pop(L,1);

            if (loc_precond)
            {
                loc_res.push_back(config);
            }
        }
    }

    return loc_res;
}

bool UD::MinigameManager::GetMinigameCondition(RE::Actor* a_actor, RE::Actor* a_helper, RE::TESObjectARMO* a_id, MinigameSetting a_setting)
{
    DEBUG("GetMinigameCondition called")
    auto L = GetMinigameScript(a_setting);
    if (!L) return false;

    DeviceObj loc_device = PapyrusDelegate::GetSingleton()->FindDeviceScriptID(a_actor,a_id);
    
    lua_getglobal(L,"Condition");
    Lua::PushTable(L,
    {
        {"Wearer",a_actor},
        {"Helper",a_helper},
        {"ID",a_id},
        {"RD",loc_device.first},
        {"DeviceObj",loc_device.second.get()},
        {"Json",a_setting->json.get()}
    });
    lua_pcall(L,1,1,0);
    const auto loc_cond = lua_toboolean(L,-1);
    lua_pop(L,1);
    return loc_cond;
}

bool UD::MinigameManager::StartMinigame(MinigameSetting a_setting, RE::Actor* a_actor, RE::Actor* a_helper, RE::TESObjectARMO* a_id)
{
    if (!a_id || !a_setting) return false;

    DEBUG("Starting minigame {} for {}",a_setting->config.name,a_id->GetName())
    auto L = GetMinigameScript(a_setting);
    if (!L)
    {
        DEBUG("Error reading script {}. Probably runtime error on init of script",a_setting->config.script)
        return false;
    }

    static int loc_cntr = 0;

    DeviceObj loc_device = PapyrusDelegate::GetSingleton()->FindDeviceScriptID(a_actor,a_id);

    MinigameData loc_data;
    loc_data.Wearer = a_actor;
    loc_data.Helper = a_helper;
    loc_data.Device.obj = loc_device.second;
    loc_data.Device.id = a_id;
    loc_data.Device.rd = loc_device.first;
    loc_data.id = loc_cntr;
    loc_data.Setting = a_setting;
    loc_cntr++;

    _minigames.push_back(MinigameDataPtr(new MinigameData(loc_data)));

    auto loc_apiptr = PRISMA_UI_API::RequestPluginAPI();
    PrismaUI = reinterpret_cast<PRISMA_UI_API::IVPrismaUI1*>(loc_apiptr);
        
    lua_getglobal(L,"OnStart");
    PushMinigameData(L,loc_data);
    lua_pcall(L,1,0,0);

    return true;
}

bool UD::MinigameManager::GetMinigameById(uint32_t a_id,UD::MinigameSetting& a_output)
{
    for(auto&& [key,setting] : _jsoncache)
    {
        if (setting->id == a_id)
        {
            a_output = setting;
            return true;
        }
    }
    return false;
}

UD::MinigameDataPtr UD::MinigameManager::GetMinigameDataById(uint32_t a_id)
{
    for(auto&& it : _minigames)
    {
        if (it->id == a_id)
        {
            return it;
        }
    }
    return nullptr;
}

void UD::MinigameManager::Update(float a_delta)
{
    for (auto&& it : _minigames)
    {
        UpdateMinigame(*it,a_delta);
    }
}

void UD::MinigameManager::SetMinigameState(MinigameState a_state)
{
    //_data.State = a_state;
}

void UD::MinigameManager::StopMinigame(int a_id)
{
    _minigames.erase(std::find_if(_minigames.begin(),_minigames.end(),[a_id](MinigameDataPtr& data)
    {
        if (data->id == a_id) return true;
        return false;
    }));
}

void UD::MinigameManager::OpenMinigameUI(int a_id)
{
    auto loc_data = GetMinigameDataById(a_id);
    if (loc_data)
    {
        auto loc_apiptr = PRISMA_UI_API::RequestPluginAPI();
        PrismaUI = reinterpret_cast<PRISMA_UI_API::IVPrismaUI1*>(loc_apiptr);
        
        if (PrismaUI)
        {
            _view = PrismaUI->CreateView(loc_data->Setting->config.uiobject.c_str(),[](PrismaView view) -> void
            {
                DEBUG("Minigame DOM is ready {}", view);
                MinigameManager::GetSingleton()->SetViewReady();
            });
        }
    }
}

void UD::MinigameManager::CloseMinigameUI(int a_id)
{
    auto loc_data = GetMinigameDataById(a_id);
    if (loc_data)
    {
        auto loc_apiptr = PRISMA_UI_API::RequestPluginAPI();
        PrismaUI = reinterpret_cast<PRISMA_UI_API::IVPrismaUI1*>(loc_apiptr);
        
        if (PrismaUI)
        {
            PrismaUI->Destroy(_view);
            _view = 0;
            _viewReady = false;
        }
    }
}

void UD::MinigameManager::InvokeUI(std::string a_command)
{
    if (_viewReady)
    {
        PrismaUI->Invoke(_view,a_command.c_str());
    }
}

void UD::MinigameManager::CheckActionCallback(uint32_t a_dxcode)
{
    for (auto&& it1 : _minigames)
    {
        for(auto&& it2 : it1->Controls)
        {
            if (it2.control.codekeyboard == a_dxcode)
            {
                //DEBUG("Calling callback")
                auto L = _scripts[it1->Setting->config.script];
                lua_getglobal(L,it2.callback.c_str());
                PushMinigameData(L,*it1);
                lua_pcall(L,1,0,0);
            }
            /* TODO: Gamepad support*/
        }
    }


}

void UD::MinigameManager::SendPapCallback(int a_id, std::string a_callback, VariableValue& a_var)
{
    auto loc_data = GetMinigameDataById(a_id);
    auto L = GetMinigameScriptById(a_id);
    lua_getglobal(L,a_callback.c_str());
    PushMinigameData(L,*loc_data);
    /* TODO: Add res as argument to callback */
    lua_pcall(L,1,0,0);
}

lua_State* UD::MinigameManager::GetMinigameScriptById(int a_id)
{
    auto loc_data = GetMinigameDataById(a_id);
    auto L = _scripts[loc_data->Setting->config.script];
    return L;
}

UD::MinigameCallback UD::MinigameManager::ParseCallback(std::string a_callback)
{
    static const std::regex loc_ParseRegex(R"((?:(.+)::(.+)\((.*)\))*)");
    MinigameCallback loc_res;
    loc_res.Module     = std::regex_replace(a_callback, loc_ParseRegex, "$1");
    std::transform(loc_res.Module.begin(), loc_res.Module.end(), loc_res.Module.begin(), ::tolower);
    loc_res.Callback   = std::regex_replace(a_callback, loc_ParseRegex, "$2");
    loc_res.Argument   = std::regex_replace(a_callback, loc_ParseRegex, "$3");
    return loc_res;
}

void UD::MinigameManager::InitConfig(MinigameSetting a_config)
{
    if (a_config && a_config->json)
    {
        try
        {
            a_config->config.name         = a_config->json->get_optional<std::string>("name").get_value_or("MISSINGNAME");
            a_config->config.description  = a_config->json->get_optional<std::string>("description").get_value_or("MISSINGDESC");
            a_config->config.uiobject     = a_config->json->get_optional<std::string>("uiobject").get_value_or("");
            a_config->config.script       = (a_config->json->get_optional<std::string>("script").get_value_or(""));
        }
        catch(const std::exception& e)
        {
            ERROR("Error initiating minigame config from json - {}!",e.what())
            return;
        };

        const std::string loc_script = a_config->config.script;
        if ((loc_script != "") && (_scripts.find(loc_script) == _scripts.end()))
        {
            lua_State* L = Lua::OpenScript(a_config->config.script.c_str());
            if (L)
            {
                DEBUG("Script {} initiated",loc_script)
                _scripts[loc_script] = L;
            }
        }
        DEBUG("Minigame config [{}] initiated, Script = {}, Ui = {}",a_config->config.name,a_config->config.script,a_config->config.uiobject)
    }
}

lua_State* UD::MinigameManager::GetMinigameScript(MinigameSetting a_config)
{
    for(auto&& it : _scripts)
    {
        if (it.first == a_config->config.script) return it.second;
    }
    return nullptr;
}

void UD::MinigameManager::UpdateMinigame(MinigameData& a_data, float a_delta)
{
    auto L = GetMinigameScript(a_data.Setting);
    if (!L) return;
    lua_getglobal(L,"OnUpdate");
    PushMinigameData(L,a_data);
    lua_pushnumber(L,a_delta);
    lua_pcall(L,2,0,0);
}

void UD::MinigameManager::PushMinigameData(lua_State* L, MinigameData& a_data)
{
    Lua::PushTable(L,
    {
        {"Wearer",a_data.Wearer},
        {"Helper",a_data.Helper},
        {"ID",a_data.Device.id},
        {"RD",a_data.Device.rd},
        {"DeviceObj",a_data.Device.obj.get()},
        {"Json",a_data.Setting->json.get()},
        {"MinigameId",(lua_Integer)a_data.id}
    });
}
