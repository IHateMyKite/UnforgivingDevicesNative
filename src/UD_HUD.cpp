
#include "UD_HUD.h"
#include "UD_ModuleManager.h"

SINGLETONBODY(UD::HudManager)

PRISMA_UI_API::IVPrismaUI1* UD::HudManager::PrismaUI = nullptr;

void UD::HudManager::Reload()
{
    auto loc_apiptr = PRISMA_UI_API::RequestPluginAPI();
    PrismaUI = reinterpret_cast<PRISMA_UI_API::IVPrismaUI1*>(loc_apiptr);

    _elements.clear();

    if (!_init || Config::GetSingleton()->GetVariable<bool>("Data.bReloadCache",false))
    {
        _init = true;
        _jsoncache.clear();

        for(auto&& [name,script] : _scripts) if (script) lua_close(script);
        _scripts.clear();

        if (_view)
        {
            PrismaUI->Destroy(_view);
            _view = NULL;
        }


        std::string loc_devconfpath = RelToAbsPath("UD\\Hud");
        std::regex loc_regex(R"regex(.*\\(.*\.[jJ][sS][oO][nN]))regex");
        uint32_t loc_id = 0;
        try
        {
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
    
                    auto loc_config = std::shared_ptr<HudElementConfigJson>(new HudElementConfigJson{loc_id++,loc_json,HudElementConfigStatus::sOK,"OK"});
                    if (InitConfig(loc_config))
                    {
                        DEBUG("Hud config {} initiated",loc_name)
                        _jsoncache[loc_name] = loc_config;
                    }
                }
            }

            // Set bases
            for(auto [path,setting] : _jsoncache)
            {
                if (!setting.get())
                {
                    ERROR("Error reading setting for {}",path)
                    continue;
                }

                auto loc_baseStr = setting->config.base;

                DEBUG("Checking {} base {}",path,loc_baseStr)

                if (loc_baseStr != "" && _jsoncache.find(loc_baseStr) != _jsoncache.end())
                {
                    auto loc_base = _jsoncache[loc_baseStr];
                    if (loc_base) 
                    {
                        setting->base = loc_base;
                        DEBUG("{} base set to {}",setting->config.name,setting->base->config.name)
                    }
                }
            }

            // Open scripts
            for(auto&& [path,setting] : _jsoncache)
            {
                OpenScript(setting);
            }
        }
        catch(...)
        {
            ERROR("Error reading hud folder")
        }
    
        DEBUG("=== Loaded Hud config files ===")
        for (auto&& [name,file] : _jsoncache)
        {
            DEBUG("\t{} - {} / {}",name, file->status, file->error)
        }

        if (PrismaUI)
        {
            _view = PrismaUI->CreateView("UD/Hud/Hud.html",[](PrismaView view) -> void
            {
                DEBUG("Hud DOM is ready {}", view);
                HudManager::PrismaUI->Show(view);
                HudManager::GetSingleton()->SetViewReady();
            });
        }
    }
}

void UD::HudManager::Update(float a_delta)
{
    _checktimer += a_delta;

    if (!_viewReady || !ModuleManager::GetSingleton()->IsReady(true)) return;

    const bool loc_menuOpen = Utility::IsBlockingMenuOpen();

    if (loc_menuOpen && _HudState == HudState::eShown)
    {
        PrismaUI->Hide(_view);
        _HudState = HudState::eHidden;
    }
    else if (!loc_menuOpen && _HudState == HudState::eHidden)
    {
        PrismaUI->Show(_view);
        _HudState = HudState::eShown;
    }

    if (_HudState == HudState::eShown)
    {
        if (_checktimer >= 1.0f)
        {
            CheckShowElements(a_delta);
            CheckHideElements(a_delta);
            _checktimer = 0.0f;
        }
        UpdateElements(a_delta);
    }
}

bool UD::HudManager::InitConfig(HudElementSetting a_config)
{
    if (a_config && a_config->json)
    {
        try
        {
            a_config->config.name         = a_config->json->get_optional<std::string>("name").get_value_or("MISSINGNAME");
            a_config->config.description  = a_config->json->get_optional<std::string>("description").get_value_or("MISSINGDESC");
            a_config->config.script       = a_config->json->get_optional<std::string>("script").get_value_or("");
            a_config->config.base         = a_config->json->get_optional<std::string>("base").get_value_or("");
            a_config->config.abstract     = a_config->json->get_optional<bool>("abstract").get_value_or(false);
            
            a_config->config.priority     = a_config->json->get_optional<int>("priority").get_value_or(0);

            auto loc_includes = a_config->json->get_child_optional("includes");
            if (loc_includes)
            {
                a_config->config.includes.clear();
                for(auto&& it : loc_includes.get())
                {
                    string loc_include = it.second.get_value_optional<std::string>().get_value_or("");
                    if (loc_include != "")
                    {
                        a_config->config.includes.push_back(loc_include);
                    }
                }
            }
        }
        catch(const std::exception& e)
        {
            ERROR("Error initiating Hud config from json - {}!",e.what())
            return false;
        };

        DEBUG("Hud config [{}] initiated, Script = {}",a_config->config.name,a_config->config.script)
        return true;
    }
    ERROR("Error reading Hud config")
    return false;
}

bool UD::HudManager::OpenScript(HudElementSetting a_config)
{
    const std::string loc_script = a_config->config.script;
    if ((loc_script != "") && (_scripts.find(loc_script) == _scripts.end()))
    {
        // Calculate includes from bases
        std::vector<string> loc_includes;
        HudElementSetting loc_base = a_config;
        while(loc_base)
        {
            std::vector<string> loc_baseincludes = loc_base->config.includes;
            std::reverse(loc_baseincludes.begin(),loc_baseincludes.end());
            loc_includes.append_range(loc_baseincludes);

            loc_base = loc_base->base;
            if (loc_base) loc_includes.push_back(loc_base->config.script);
        }

        std::reverse(loc_includes.begin(),loc_includes.end());

        lua_State* L = Lua::OpenScript(a_config->config.script.c_str(),loc_includes);
        if (L)
        {
            DEBUG("Hud Script {} initiated",loc_script)
            _scripts[loc_script] = L;
        }
        else
        {
            ERROR("Error initiating Hud script for {}",a_config->config.name)
            return false;
        }
    }

    DEBUG("Hud script for [{}] opened, Script = {}",a_config->config.name,a_config->config.script)

    return true;
}

void UD::HudManager::PushHudData(lua_State* L, HudElementData& a_data)
{
    Lua::PushTable(L,
    {
        {"Target",a_data.Target},
        {"Json",a_data.Setting->json.get()},
        {"Id",(lua_Integer)a_data.id}
    });
}

lua_State* UD::HudManager::GetElementScript(HudElementSetting a_setting)
{
    lua_State* L = NULL;

    HudElementSetting loc_setting = a_setting;

    while (!L && loc_setting)
    {
        L = _scripts[loc_setting->config.script];
        loc_setting = loc_setting->base;
    }

    return L;
}

void UD::HudManager::CheckShowElements(float a_delta)
{
    // Check if meter should be shown
    for(auto&& [name,setting] : _jsoncache)
    {
        if (setting->config.abstract) continue;

        if (std::find_if(_elements.begin(),_elements.end(),[setting](const HudElementDataPtr& data)
        {
            return data->Setting->json == setting->json;
        }) != _elements.end())
        {
            // Already open, continue
            continue;
        }

        lua_State* L = GetElementScript(setting);
        
        if (!L)
        {
            ERROR("Cant find script for element {}",setting->config.name)
            continue;
        }

        bool loc_cond = false;

        if (lua_getglobal(L,"Condition") != LUA_TNIL)
        {
            Lua::PushTable(L,
            {
                {"Target",RE::PlayerCharacter::GetSingleton()},
                {"Json",setting->json.get()}
            });

            auto loc_luares = lua_pcall(L,1,1,0);
            if (loc_luares != LUA_OK)
            {
                ERROR("Error running function Condition - {}",loc_luares)
                continue;
            }
            loc_cond = lua_toboolean(L,-1);
            lua_pop(L,1);
        }
        else
        {
            ERROR("Can't find function Condition on Element {}",setting->config.name)
        }

        if (loc_cond)
        {
            HudElementData* loc_data = new HudElementData();
            loc_data->id = _hudcntr;
            _hudcntr++;
            loc_data->Target = RE::PlayerCharacter::GetSingleton();
            loc_data->Setting = setting;
            loc_data->State = HudElementState::eShown;
            HudElementDataPtr loc_dataPtr = HudElementDataPtr(loc_data);
            _elements.push_back(loc_dataPtr);

            if (lua_getglobal(L,"Show") != LUA_TNIL)
            {
                PushHudData(L,*loc_data);

                auto loc_luares = lua_pcall(L,1,0,0);
                if (loc_luares != LUA_OK)
                {
                    ERROR("Error running function Show - {}",loc_luares)
                }
            }
        }
    }
}

void UD::HudManager::CheckHideElements(float a_delta)
{
    std::vector<HudElementDataPtr> loc_toremove;
    for(auto&& it : _elements)
    {
        lua_State* L = GetElementScript(it->Setting);
        if (!L)
        {
            ERROR("Cant find script for element {}",it->Setting->config.name)
            continue;
        }

        bool loc_cond = false;

        if (lua_getglobal(L,"Condition") != LUA_TNIL)
        {
            PushHudData(L,*it);

            auto loc_luares = lua_pcall(L,1,1,0);
            if (loc_luares != LUA_OK)
            {
                ERROR("Error running function Condition - {}",loc_luares)
                continue;
            }

            loc_cond = lua_toboolean(L,-1);

            lua_pop(L,1);

            if (!loc_cond)
            {
                loc_toremove.push_back(it);
            }
        }
        else
        {
            ERROR("Can't find function Condition on Element {}",it->Setting->config.name)
        }

    }

    for(auto&& it : loc_toremove)
    {
        auto loc_el = std::find(_elements.begin(),_elements.end(),it);

        lua_State* L = GetElementScript(it->Setting);

        if (lua_getglobal(L,"Hide") != LUA_TNIL)
        {
            PushHudData(L,*it);

            auto loc_luares = lua_pcall(L,1,0,0);
            if (loc_luares != LUA_OK)
            {
                ERROR("Error running function Hide - {}",loc_luares)
            }
        }
        _elements.erase(loc_el);
    }
}

void UD::HudManager::UpdateElements(float a_delta)
{
    for(auto&& it : _elements)
    {
        lua_State* L = GetElementScript(it->Setting);

        if (lua_getglobal(L,"Update") != LUA_TNIL)
        {
            PushHudData(L,*it);
            lua_pushnumber(L,a_delta);

            auto loc_luares = lua_pcall(L,2,0,0);
            if (loc_luares != LUA_OK)
            {
                ERROR("Error running function Update - {}",loc_luares)
            }
        }
    }
}

void UD::HudManager::InvokeHud(std::string a_message)
{
    if (_viewReady)
    {
        PrismaUI->Invoke(_view,a_message.c_str());
    }
}
