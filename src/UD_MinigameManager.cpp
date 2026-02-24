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
    
                auto loc_config = std::shared_ptr<MinigameConfigJson>(new MinigameConfigJson{loc_json,MinigameConfigStatus::sOK,"OK"});
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

std::vector<std::string> UD::MinigameManager::GetListOfMinigames(RE::Actor* a_actor, RE::TESObjectARMO* a_id)
{
    DEBUG("GetListOfMinigames called")
    std::vector<std::string> loc_res;

    Object loc_device = PapyrusDelegate::GetSingleton()->FindDeviceScriptID(a_actor,a_id);

    if (a_actor && loc_device)
    {
        DEBUG("GetListOfMinigames - Device found")

        for (auto&& [path,config] : _jsoncache)
        {
            auto L = _scripts[config->config.script];
            if (!L) continue;

            Lua::LuaContext loc_context;
            loc_context.Wearer      = a_actor;
            loc_context.Helper      = nullptr;
            loc_context.DeviceObj   = loc_device;

            lua_getglobal(L,"Precondition");
            lua_pushlightuserdata(L,&loc_context);

            lua_pcall(L,1,1,0);
            const auto loc_precond = lua_toboolean(L,-1);
            lua_pop(L,1);
            DEBUG("Precondition = {}",loc_precond)

            if (loc_precond)
            {
                loc_res.push_back(config->config.name);
            }
        }
    }

    return loc_res;
}

bool UD::MinigameManager::StartMinigame(RE::BGSBaseAlias* a_minigame, RE::Actor* a_actor, RE::TESObjectARMO* a_rd)
{
    if (_data.State != MinigameState::eNotStarted) return false;

    const auto loc_vm = InternalVM::GetSingleton();
    RE::BSTSmartPointer<RE::BSScript::Object> loc_object;
    const auto loc_handle = loc_vm->GetObjectHandlePolicy()->GetHandleForObject(a_minigame->GetVMTypeID(),a_minigame);
    const bool loc_found = loc_vm->FindBoundObject(loc_handle, "ud_minigame", loc_object);
    if (loc_found)
    {
        std::string loc_path = Utility::GetPropertyString(loc_object,"UIPath",false,"");
        if (PrismaUI)
        {
            SetMinigameState(MinigameState::eStarting);
            _view = PrismaUI->CreateView(loc_path.c_str(),[](PrismaView view) -> void
            {
                DEBUG("View DOM is ready {}", view);
                MinigameManager::GetSingleton()->SetMinigameState(MinigameState::eRunning);
                PrismaUI->Focus(view,false,false);
            });

            PrismaUI->RegisterJSListener(_view, "SendCallback", [](const char* a_arg)
            {
                std::string loc_str = a_arg;
                MinigameCallback loc_callback = MinigameManager::GetSingleton()->ParseCallback(loc_str);
                //MinigameManager::GetSingleton()->SendCallback(loc_callback);
            });

            return true;
        }
    }
    return false;
}

//void UD::MinigameManager::SendCallback(MinigameCallback a_callback)
//{
//        if (a_callback.Module != "")
//        {
//            const auto loc_vm = InternalVM::GetSingleton();
//            RE::BSTSmartPointer<RE::BSScript::IStackCallbackFunctor> loc_callback;
//
//            if (a_callback.Module == "this")
//            {
//                auto loc_args = new RE::BSScript::FunctionArguments<void, RE::Actor*, std::string>(std::forward<RE::Actor*>(_data.Helper),std::forward<std::string>(a_callback.Argument));
//                loc_vm->DispatchMethodCall(_data.DeviceObj,a_callback.Callback,loc_args,loc_callback);
//            }
//            else
//            {
//                auto loc_module = ModuleManager::GetSingleton()->GetModuleObjectByAlias(a_callback.Module);
//                if (loc_module && loc_module->object)
//                {
//                    auto loc_args = new RE::BSScript::FunctionArguments<void, RE::Actor*, RE::Actor*, RE::TESObjectARMO*, std::string>(
//                    std::forward<RE::Actor*>(_data.Wearer),
//                    std::forward<RE::Actor*>(_data.Helper),
//                    std::forward<RE::TESObjectARMO*>((RE::TESObjectARMO*)Utility::GetPropertyObject(_data.DeviceObj,"DeviceInventory",false,RE::TESObjectARMO::FORMTYPE)),
//                    std::forward<std::string>(a_callback.Argument));
//                    loc_vm->DispatchMethodCall(loc_module->object,a_callback.Callback,loc_args,loc_callback);
//                }
//                else
//                {
//                    ERROR("Can't find module {}",a_callback.Module)
//                }
//            }
//        }
//        else
//        {
//            // Void Callback, dont dispatch callback
//        }
//}

void UD::MinigameManager::SetMinigameState(MinigameState a_state)
{
    _data.State = a_state;
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

void UD::MinigameManager::InitConfig(std::shared_ptr<MinigameConfigJson> a_config)
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

void UD::MinigameManager::CreateContext(RE::Actor* a_actor, RE::Actor* a_helper, RE::TESObjectARMO* a_id)
{
    
}
