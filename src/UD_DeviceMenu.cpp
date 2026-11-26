#include <UD_DeviceMenu.h>
#include <UD_ModuleManager.h>
#include <UD_Utility.h>
#include <UD_MinigameManager.h>
#include <UD_DeviceManager.h>
#include <UD_VariableManager.h>

SINGLETONBODY(UD::DeviceMenu)

bool UD::DeviceMenu::ViewReady = false;
bool UD::DeviceMenu::ViewReadySingle = false;

namespace UD
{
    void DeviceMenu::Reload()
    {
        DEBUG("Reload called")
        ViewReady = false;
        ViewReadySingle = false;

        if (_view)
        {
            UIManager::GetSingleton()->DestroyView(_view);
            _view = 0x0UL;
        }

        if (_viewSingle)
        {
            UIManager::GetSingleton()->DestroyView(_viewSingle);
            _viewSingle = 0x0UL;
        }

        std::thread([this]
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(3000));
            SKSE::GetTaskInterface()->AddTask([this]
            {
                if (_view == 0x0UL && _viewSingle == 0x0UL)
                {
                    // Listen for UI events
                    static auto loc_exit = [](const char* a_type)
                    {
                        DEBUG("ExitMenu({}) called",a_type)
                        DeviceMenu::GetSingleton()->HideMenu((UIMenu)std::stoi(a_type));
                    };

                    static auto loc_sendcallback = [](const char* a_arg)
                    {
                        std::string loc_str = a_arg;
                        std::vector<std::string> loc_indx;

                        try
                        {
                            boost::split(loc_indx,loc_str,boost::is_any_of(","));
                        }
                        catch(...)
                        {
                            ERROR("Error spliting argument received by SendCallback - {}",a_arg)
                            return;
                        }
                
                        if (loc_indx.size() == 2)
                        {
                            DeviceMenu::GetSingleton()->SendCallback(std::stoi(loc_indx[0]),std::stoi(loc_indx[1]));
                        }
                        else
                        {
                            ERROR("Incorrect number of indexes received for SendCallback!")
                        }
                    };

                    static auto loc_startminigame = [](const char* a_arg)
                    {
                        DEBUG("StartMinigame({}) called",a_arg)
                        std::string loc_str = a_arg;
                        std::vector<std::string> loc_indx;

                        try
                        {
                            boost::split(loc_indx,loc_str,boost::is_any_of(","));
                        }
                        catch(...)
                        {
                            ERROR("Error spliting argument received by SendCallback - {}",a_arg)
                            return;
                        }
                
                        if (loc_indx.size() == 3)
                        {
                            DeviceMenu::GetSingleton()->StartMinigame(std::stoi(loc_indx[0]),std::stoi(loc_indx[1]),loc_indx[2]);
                        }
                        else
                        {
                            ERROR("Incorrect number of indexes received for StartMinigame!")
                        }
                    };

                    _view = UIManager::GetSingleton()->CreateView("DeviceMenu","DeviceMenu/DeviceMenu.html",[](ViewId view) -> void
                    {
                        UIManager::GetSingleton()->RegisterConsoleCallback(view,ConsoleCallback);
                        UIManager::GetSingleton()->ShowView(view,false);
                        UIManager::GetSingleton()->RegisterJSListener(view, "ExitMenu", loc_exit);
                        UIManager::GetSingleton()->RegisterJSListener(view, "SendCallback", loc_sendcallback);
                        UIManager::GetSingleton()->RegisterJSListener(view, "StartMinigame", loc_startminigame);
                        ViewReady = true;
                        DEBUG("DeviceMenu ready {}", view);
                    },UILayer::Popup,false);

                    _viewSingle = UIManager::GetSingleton()->CreateView("DeviceMenuSingle","DeviceMenu/DeviceMenuSingle.html",[](ViewId view) -> void
                    {
                        UIManager::GetSingleton()->RegisterConsoleCallback(view,ConsoleCallback);
                        UIManager::GetSingleton()->ShowView(view,false);
                        UIManager::GetSingleton()->RegisterJSListener(view, "ExitMenu", loc_exit);
                        UIManager::GetSingleton()->RegisterJSListener(view, "SendCallback", loc_sendcallback);
                        UIManager::GetSingleton()->RegisterJSListener(view, "StartMinigame", loc_startminigame);
                        ViewReadySingle = true;
                        DEBUG("DeviceMenuSingle ready {}", view);
                    },UILayer::Popup,false);
                }
            });
        }).detach();
    }

    void DeviceMenu::Update()
    {

    }

    bool DeviceMenu::ShowDeviceMenu(RE::Actor* a_actor, RE::Actor* a_helper, std::vector<std::string> a_callbacks)
    {
        DEBUG("ShowDeviceMenu() called")
        if (ViewReady && _currentMenu == eNone && a_actor)
        {
            _devMenuData.Callbacks = ParseCallbacks(a_callbacks);
            _devMenuData.List.clear();
            _devMenuData.Wearer = a_actor;
            _devMenuData.Helper = a_helper;

            UIManager::GetSingleton()->ShowView(_view,true);
            UIManager::GetSingleton()->FocusView(_view,true);

            auto loc_Devices = PapyrusDelegate::GetSingleton()->FindAllDeviceScripts(a_actor);
            DEBUG("ShowDeviceMenu() - Showing list of {} devices",loc_Devices.size())

            std::vector<std::string> loc_devicesList;
            for (auto&& [loc_rd,loc_device] : loc_Devices)
            {
                AddDeviceToList(a_actor,a_helper,loc_rd,loc_device,loc_devicesList);
            }
            std::string loc_devicestr = "[" + boost::join(loc_devicesList,",") + "]";

            std::string loc_arg = "{";
            loc_arg += "wearer: \"" + std::string(a_actor->GetName()) + "\",";
            loc_arg += "helper: \"" + (a_helper ? std::string(a_helper->GetName()) : "none") + "\",";
            loc_arg += "arousal: " + std::to_string(ORS::OrgasmManager::GetSingleton()->GetOrgasmVariable(a_actor,ORS::OrgasmVariable::vArousal)) + ",";
            loc_arg += "orgasm: " + std::to_string(ORS::OrgasmManager::GetSingleton()->GetOrgasmProgress(a_actor,1)*100.0f) + ",";

            std::vector<std::string> loc_buttonNames(_devMenuData.Callbacks.size());
            for (int i =0; i < loc_buttonNames.size(); i++)
            {
                std::string tmp_callback = std::format("{{name: \"{}\",module: \"{}\"}}",_devMenuData.Callbacks[i].Name,_devMenuData.Callbacks[i].Module);
                loc_buttonNames[i] = tmp_callback;
            }
            loc_arg += "callbacks: [" + boost::join(loc_buttonNames,",") + "],";

            CreateActorMinigames(loc_arg,true,a_actor,a_helper);

            loc_arg += "devices: " + loc_devicestr;
            loc_arg += "}";
            std::string loc_call = std::format("InitDeviceList({})",loc_arg);
            DEBUG("ShowDeviceMenu() - Sending {}",loc_call)
            UIManager::GetSingleton()->Invoke(_view,loc_call);

            _currentMenu = UIMenu::eDeviceMenu;
            return true;
        }
        return false;
    }

    bool DeviceMenu::ShowDeviceMenuSingle(RE::TESObjectARMO* a_id, RE::TESObjectARMO* a_rd, RE::Actor* a_actor, RE::Actor* a_helper, std::vector<std::string> a_callbacks)
    {
        DEBUG("ShowDeviceMenuSingle() called")
        if (ViewReadySingle && _currentMenu == eNone && a_actor && (a_id || a_rd))
        {
            _devMenuData.Callbacks = ParseCallbacks(a_callbacks);
            _devMenuData.List.clear();
            _devMenuData.Wearer = a_actor;
            _devMenuData.Helper = a_helper;

            UIManager::GetSingleton()->ShowView(_viewSingle,true);
            UIManager::GetSingleton()->FocusView(_viewSingle,true);

            auto loc_Devices = PapyrusDelegate::GetSingleton()->FindAllDeviceScripts(a_actor);

            std::vector<std::string> loc_devicesList;
            for (auto&& [loc_rd,loc_device] : loc_Devices)
            {
                bool loc_add = a_rd && a_rd == loc_rd;
                if (!loc_add && a_id)
                {
                    RE::TESObjectARMO* loc_id = (RE::TESObjectARMO*)Utility::GetPropertyObject(loc_device,"DeviceInventory",false,(RE::VMTypeID)RE::TESObjectARMO::FORMTYPE);
                    loc_add = loc_id == a_id;
                }
                if (loc_add) AddDeviceToList(a_actor,a_helper,loc_rd,loc_device,loc_devicesList);
            }
            std::string loc_devicestr = "[" + boost::join(loc_devicesList,",") + "]";
            std::string loc_arg = "{";
            loc_arg += "wearer: \"" + std::string(a_actor->GetName()) + "\",";
            loc_arg += "helper: \"" + (a_helper ? std::string(a_helper->GetName()) : "none") + "\",";
            loc_arg += "arousal: " + std::to_string(ORS::OrgasmManager::GetSingleton()->GetOrgasmVariable(a_actor,ORS::OrgasmVariable::vArousal)) + ",";
            loc_arg += "orgasm: " + std::to_string(ORS::OrgasmManager::GetSingleton()->GetOrgasmProgress(a_actor,1)*100.0f) + ",";

            std::vector<std::string> loc_buttonNames(_devMenuData.Callbacks.size());
            for (int i =0; i < loc_buttonNames.size(); i++)
            {
                std::string tmp_callback = std::format("{{name: \"{}\",module: \"{}\"}}",_devMenuData.Callbacks[i].Name,_devMenuData.Callbacks[i].Module);
                loc_buttonNames[i] = tmp_callback;
            }
            loc_arg += "callbacks: [" + boost::join(loc_buttonNames,",") + "],";

            CreateActorMinigames(loc_arg,true,a_actor,a_helper);

            loc_arg += "devices: " + loc_devicestr;
            loc_arg += "}";
            std::string loc_call = std::format("InitDeviceListSingle({})",loc_arg);
            DEBUG("ShowDeviceMenuSingle() - Sending {}",loc_call)
            UIManager::GetSingleton()->Invoke(_viewSingle,loc_call);

            _currentMenu = UIMenu::eDeviceMenuSingle;
            return true;
        }
        return false;
    }

    bool DeviceMenu::IsMenuOpen()
    {
        return _currentMenu != eNone;
    }

    void DeviceMenu::HideMenu(UIMenu arg_type)
    {
        DEBUG("HideMenu({}) called",arg_type)
        _currentMenu = UIMenu::eNone;

        switch(arg_type)
        {
            case UIMenu::eDeviceMenu:
                UIManager::GetSingleton()->FocusView(_view,false);
                UIManager::GetSingleton()->ShowView(_view,false);
                _devMenuData.List.clear();
                _devMenuData = DeviceMenuData();
            break;
            case UIMenu::eDeviceMenuSingle:
                UIManager::GetSingleton()->FocusView(_viewSingle,false);
                UIManager::GetSingleton()->ShowView(_viewSingle,false);
                _devMenuData.List.clear();
                _devMenuData = DeviceMenuData();
            break;
            default:
            break;
        }
    }

    void DeviceMenu::SendCallback(int a_indxDev, int a_indxCall)
    {
        if (_devMenuData.Callbacks[a_indxCall].Module != "")
        {
            const auto loc_vm = InternalVM::GetSingleton();
            RE::BSTSmartPointer<RE::BSScript::IStackCallbackFunctor> loc_callback;

            if (_devMenuData.Callbacks[a_indxCall].Module == "this")
            {
                auto loc_args = new RE::BSScript::FunctionArguments<void, RE::Actor*, std::string>(std::forward<RE::Actor*>(_devMenuData.Helper),std::forward<std::string>(_devMenuData.Callbacks[a_indxCall].Argument));
                loc_vm->DispatchMethodCall(_devMenuData.List[a_indxDev].obj,_devMenuData.Callbacks[a_indxCall].Callback,loc_args,loc_callback);
            }
            else
            {
                auto loc_module = ModuleManager::GetSingleton()->GetModuleObjectByAlias(_devMenuData.Callbacks[a_indxCall].Module);
                if (loc_module && loc_module->object)
                {
                    auto loc_args = new RE::BSScript::FunctionArguments<void, RE::Actor*, RE::Actor*, RE::TESObjectARMO*, std::string>(
                    std::forward<RE::Actor*>(_devMenuData.Wearer),
                    std::forward<RE::Actor*>(_devMenuData.Helper),
                    std::forward<RE::TESObjectARMO*>(a_indxDev >= 0 ? (RE::TESObjectARMO*)Utility::GetPropertyObject(_devMenuData.List[a_indxDev].obj,"DeviceInventory",false,RE::TESObjectARMO::FORMTYPE):nullptr),
                    std::forward<std::string>(_devMenuData.Callbacks[a_indxCall].Argument));
                    loc_vm->DispatchMethodCall(loc_module->object,_devMenuData.Callbacks[a_indxCall].Callback,loc_args,loc_callback);
                }
                else
                {
                    ERROR("Can't find module {}",_devMenuData.Callbacks[a_indxCall].Module)
                }
            }
        }
        else
        {
            // Void Callback, dont dispatch callback
        }
        HideMenu(_currentMenu);
    }

    void DeviceMenu::StartMinigame(int a_indxDev, int a_minId, string a_cntx)
    {
        
        MinigameSetting loc_min;
        if (MinigameManager::GetSingleton()->GetMinigameById(a_minId,loc_min))
        {
            if(a_indxDev != -1)
            {
                auto loc_dev =  _devMenuData.List[a_indxDev];
                MinigameManager::GetSingleton()->StartMinigame(loc_min,_devMenuData.Wearer,_devMenuData.Helper,loc_dev.id,a_cntx);
            }
            else
            {
                MinigameManager::GetSingleton()->StartMinigame(loc_min,_devMenuData.Wearer,_devMenuData.Helper,a_cntx);
            }

        }
        HideMenu(_currentMenu);
    }

    std::vector<ButtonCallback> DeviceMenu::ParseCallbacks(std::vector<std::string> a_callbacks)
    {
        static const std::regex loc_ParseRegex(R"(\[(.+)\](?:(.+)::(.+)\((.*)\))*)");
        std::vector<ButtonCallback> loc_res(a_callbacks.size());
        for (int i = 0; i < a_callbacks.size(); i++)
        {
            ButtonCallback loc_ballback;
            loc_ballback.Name       = std::regex_replace(a_callbacks[i], loc_ParseRegex, "$1");
            loc_ballback.Module     = std::regex_replace(a_callbacks[i], loc_ParseRegex, "$2");
            std::transform(loc_ballback.Module.begin(), loc_ballback.Module.end(), loc_ballback.Module.begin(), ::tolower);
            loc_ballback.Callback   = std::regex_replace(a_callbacks[i], loc_ParseRegex, "$3");
            loc_ballback.Argument   = std::regex_replace(a_callbacks[i], loc_ParseRegex, "$4");
            loc_res[i] = loc_ballback;
        }
        
        return loc_res;
    }

    void DeviceMenu::AddDeviceToList(RE::Actor* a_wearer, RE::Actor* a_helper, RE::TESObjectARMO* a_rd, Object a_obj, std::vector<std::string>& a_list)
    {
        #undef GetObject

        auto loc_obj = a_obj;

        RE::TESObjectARMO* loc_id = (RE::TESObjectARMO*)Utility::GetPropertyObject(loc_obj,"DeviceInventory",false,(RE::VMTypeID)RE::TESObjectARMO::FORMTYPE);
        if (loc_id)
        {
            std::string loc_arg = "{";

            loc_arg += "name: \"" + std::string(loc_id->GetName()) + "\"";

            loc_arg += ",values: [";
                    
            const float loc_acc = DeviceManager::GetSingleton()->GetDeviceAccessibility(a_rd,loc_obj.get(),a_wearer,a_helper,true);
            CreateValueDetail(loc_arg, true,"Accessibility",std::format("{}%",Utility::Round(loc_acc*100.0f)),"");

            auto loc_configs = DeviceManager::GetSingleton()->GetDeviceConfigs(loc_obj);

            for (auto conf : loc_configs)
            {
                for (auto var : conf.variables)
                {
                    const std::string loc_format    = var.atributes["format"].value_or("{}");
                    const std::string loc_style     = var.atributes["style"].value_or("");
                    const std::string loc_name      = var.atributes["name"].value_or(var.name);
                    const std::string loc_convertor = var.atributes["conv"].value_or("");

                    const auto loc_value = GetVariableValue(loc_obj.get(),"thisdevice::" + var.name + "()");

                    auto loc_valueFormated = ProcessDeviceVariable(loc_value,loc_format,loc_convertor);

                    CreateValueDetail(loc_arg, true,loc_name,loc_valueFormated,loc_style);
                }
            }

            loc_arg += "]";

            RE::BSString loc_str = "";
            loc_id->GetDescription(loc_str,loc_id);
            loc_arg += ",desc: \"" + std::string(loc_str) + "\"";

            auto loc_mods = Utility::GetPropertyObjectArrayRaw(loc_obj,"UD_ModifiersRef",false);
            std::vector<std::string> loc_modlist;
            for (auto&& mod : loc_mods)
            {
                std::string loc_modname = Utility::GetPropertyString(mod,"NameFull",false,"");
                std::string loc_moddesc = Utility::GetPropertyString(mod,"Description",false,"");

                if (loc_modname != "")
                {
                    std::string loc_modclass = std::format("{{name: \"{}\",desc: \"{}\"}}",loc_modname,loc_moddesc);
                    loc_modlist.push_back(loc_modclass);
                }
            }
            std::string loc_modstr = ",mods: [" + boost::join(loc_modlist,",") + "]";
            loc_arg += loc_modstr;

            std::vector<std::string> loc_minigamelist;
            auto loc_minigames = MinigameManager::GetSingleton()->GetListOfMinigames(a_wearer,a_helper,loc_id);
            for (auto&& min : loc_minigames)
            {
                std::string loc_minname = min->config.name;
                std::string loc_mindesc = min->config.description;
                if (loc_minname != "")
                {
                    uint32_t    loc_state   = MinigameManager::GetSingleton()->GetMinigameCondition(a_wearer,a_helper,loc_id,min);
                    string      loc_context = MinigameManager::GetSingleton()->GetMinigameContexts(a_wearer,a_helper,loc_id,min);
                    if (loc_context != "")
                    {
                        auto loc_options = Utility::DeserializeJson(std::format("{}",loc_context));
                        bool loc_optionavailable = false;
                        for(auto&& [key,val] : loc_options)
                        {
                            int loc_optionstate = val.get_optional<int>("state").get_value_or(0);
                            if (loc_optionstate > 0)
                            {
                                loc_optionavailable = true;
                                break;
                            }
                        }

                        if (!loc_optionavailable) loc_state = 0;

                        std::string loc_minclass = std::format("{{name: \"{}\",desc: \"{}\",state: {},id: {},context: {}}}",loc_minname,loc_mindesc,loc_state,min->id,loc_context);
                        loc_minigamelist.push_back(loc_minclass);
                    }
                    else
                    {
                        std::string loc_minclass = std::format("{{name: \"{}\",desc: \"{}\",state: {},id: {}}}",loc_minname,loc_mindesc,loc_state,min->id);
                        loc_minigamelist.push_back(loc_minclass);
                    }
                }
            }
            std::string loc_minstr = ",minigames: [" + boost::join(loc_minigamelist,",") + "]";
            loc_arg += loc_minstr;

            const auto loc_locksjson = GetVariableValue(loc_obj.get(),"thisdevice::UD_LocksJson()");
            std::string loc_locks = ",locks: " + loc_locksjson.Value;
            loc_arg += loc_locks;

            loc_arg += "}";

            _devMenuData.List.push_back({a_obj,loc_id,a_rd});
            a_list.push_back(loc_arg);
        }
    }

    void DeviceMenu::CreateActorMinigames(std::string& a_input, bool a_sep, RE::Actor* a_wearer, RE::Actor* a_helper)
    {
        std::vector<std::string> loc_minigamelist;
        auto loc_minigames = MinigameManager::GetSingleton()->GetListOfMinigames(a_wearer,a_helper);
        for (auto&& min : loc_minigames)
        {
            std::string loc_minname = min->config.name;
            std::string loc_mindesc = min->config.description;
            if (loc_minname != "")
            {
                uint32_t    loc_state   = MinigameManager::GetSingleton()->GetMinigameCondition(a_wearer,a_helper,min);
                string      loc_context = MinigameManager::GetSingleton()->GetMinigameContexts(a_wearer,a_helper,min);
                if (loc_context != "")
                {
                    auto loc_options = Utility::DeserializeJson(std::format("{}",loc_context));
                    bool loc_optionavailable = false;
                    for(auto&& [key,val] : loc_options)
                    {
                        int loc_optionstate = val.get_optional<int>("state").get_value_or(0);
                        if (loc_optionstate > 0)
                        {
                            loc_optionavailable = true;
                            break;
                        }
                    }

                    if (!loc_optionavailable) loc_state = 0;

                    std::string loc_minclass = std::format("{{name: \"{}\",desc: \"{}\",state: {},id: {},context: {}}}",loc_minname,loc_mindesc,loc_state,min->id,loc_context);
                    loc_minigamelist.push_back(loc_minclass);
                }
                else
                {
                    std::string loc_minclass = std::format("{{name: \"{}\",desc: \"{}\",state: {},id: {}}}",loc_minname,loc_mindesc,loc_state,min->id);
                    loc_minigamelist.push_back(loc_minclass);
                }
            }
        }
        std::string loc_minstr = "minigames: [" + boost::join(loc_minigamelist,",") + "]";
        a_input += loc_minstr;
        if(a_sep) a_input += ",";
    }

    void DeviceMenu::CreateValueDetail(std::string& a_input,bool a_sep,std::string a_name, std::string a_value, std::string a_style)
    {
        std::string loc_value = std::format("{{name: \"{}\",value: \"{}\", style: \"{}\"}}",a_name,a_value,a_style);
        a_input += loc_value;
        if (a_sep) a_input += ",";
    }

}