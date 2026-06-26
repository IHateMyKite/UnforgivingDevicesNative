#include <UD_DeviceMenu.h>
#include <UD_ModuleManager.h>
#include <UD_Utility.h>
#include <UD_MinigameManager.h>
#include <UD_DeviceManager.h>
#include <UD_VariableManager.h>

SINGLETONBODY(UD::DeviceMenu)

PRISMA_UI_API::IVPrismaUI1* UD::DeviceMenu::PrismaUI = nullptr;
bool UD::DeviceMenu::ViewReady = false;

namespace UD
{
    void DeviceMenu::Reload()
    {
        DEBUG("Reload called")
        ViewReady = false;
        auto loc_apiptr = PRISMA_UI_API::RequestPluginAPI();
        PrismaUI = reinterpret_cast<PRISMA_UI_API::IVPrismaUI1*>(loc_apiptr);
        if (_view && PrismaUI)
        {
            PrismaUI->Destroy(_view);
            _view = 0x0UL;
        }

        std::thread([this]
        {
            DEBUG("Init thread called")
            std::this_thread::sleep_for(std::chrono::milliseconds(3000));
            SKSE::GetTaskInterface()->AddTask([this]
            {
                DEBUG("Init PrismaUI for DeviceMenu")
                if (PrismaUI && _view == 0x0UL)
                {
                    _view = PrismaUI->CreateView("UD/DeviceMenu.html",[](PrismaView view) -> void
                    {
                        DEBUG("View DOM is ready {}", view);
                        ViewReady = true;
                        PrismaUI->Hide(view);
                    });

                    // Listen for UI events
                    PrismaUI->RegisterJSListener(_view, "ExitMenu", [](const char* a_type)
                    {
                        DeviceMenu::GetSingleton()->HideMenu((UIMenu)std::stoi(a_type));
                    });

                    PrismaUI->RegisterJSListener(_view, "SendCallback", [](const char* a_arg)
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
                    });

                    PrismaUI->RegisterJSListener(_view, "StartMinigame", [](const char* a_arg)
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
                    });
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

            PrismaUI->Show(_view);
            PrismaUI->Focus(_view,true);

            auto loc_Devices = PapyrusDelegate::GetSingleton()->FindAllDeviceScripts(a_actor);
            DEBUG("ShowDeviceMenu() - Showing list of {} devices",loc_Devices.size())

            std::vector<std::string> loc_devicesList;
            for (auto&& [loc_rd,loc_device] : loc_Devices)
            {
                #undef GetObject

                auto loc_obj = loc_device;

                RE::TESObjectARMO* loc_id = (RE::TESObjectARMO*)Utility::GetPropertyObject(loc_obj,"DeviceInventory",false,(RE::VMTypeID)RE::TESObjectARMO::FORMTYPE);
                if (loc_id)
                {
                    std::string loc_arg = "{";

                    loc_arg += "name: \"" + std::string(loc_id->GetName()) + "\",";

                    loc_arg += "values: [";
                    
                    const float loc_acc = DeviceManager::GetSingleton()->GetDeviceAccessibility(loc_rd,loc_obj.get(),a_actor,a_helper);
                    CreateValueDetail(loc_arg, true,"Accessibility",std::format("{}%",Utility::Round(loc_acc*100.0f)),"");

                    const auto loc_configs = DeviceManager::GetSingleton()->GetDeviceConfigs(loc_obj);

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

                    loc_arg += "],";

                    RE::BSString loc_str = "";
                    loc_id->GetDescription(loc_str,loc_id);
                    loc_arg += "desc: \"" + std::string(loc_str) + "\",";

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
                    std::string loc_modstr = "[" + boost::join(loc_modlist,",") + "],";
                    loc_arg += "mods: " + loc_modstr;

                    std::vector<std::string> loc_minigamelist;
                    auto loc_minigames = MinigameManager::GetSingleton()->GetListOfMinigames(a_actor,a_helper,loc_id);
                    for (auto&& min : loc_minigames)
                    {
                        std::string loc_minname = min->config.name;
                        std::string loc_mindesc = min->config.description;
                        if (loc_minname != "")
                        {
                            uint32_t    loc_state   = MinigameManager::GetSingleton()->GetMinigameCondition(a_actor,a_helper,loc_id,min);
                            string      loc_context = MinigameManager::GetSingleton()->GetMinigameContexts(a_actor,a_helper,loc_id,min);
                            if (loc_context != "")
                            {
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
                    std::string loc_minstr = "[" + boost::join(loc_minigamelist,",") + "]";
                    loc_arg += "minigames: " + loc_minstr;

                    loc_arg += "}";

                    _devMenuData.List.push_back({loc_device,loc_id,loc_rd});
                    loc_devicesList.push_back(loc_arg);
                }

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

            loc_arg += "devices: " + loc_devicestr;
            loc_arg += "}";
            std::string loc_call = std::format("InitDeviceList({})",loc_arg);
            DEBUG("ShowDeviceMenu() - Sending {}",loc_call)
            PrismaUI->Invoke(_view,loc_call.c_str());

            _currentMenu = UIMenu::eDeviceMenu;
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
        _currentMenu = UIMenu::eNone;
        PrismaUI->Unfocus(_view);
        PrismaUI->Hide(_view);

        switch(arg_type)
        {
            case UIMenu::eDeviceMenu:
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
        HideMenu(UIMenu::eDeviceMenu);
    }

    void DeviceMenu::StartMinigame(int a_indxDev, int a_minId, string a_cntx)
    {
        auto loc_dev = _devMenuData.List[a_indxDev];
        MinigameSetting loc_min;
        if (MinigameManager::GetSingleton()->GetMinigameById(a_minId,loc_min))
        {
            MinigameManager::GetSingleton()->StartMinigame(loc_min,_devMenuData.Wearer,_devMenuData.Helper,loc_dev.id,a_cntx);
        }
        HideMenu(UIMenu::eDeviceMenu);
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

    void DeviceMenu::CreateValueDetail(std::string& a_input,bool a_sep,std::string a_name, std::string a_value, std::string a_style)
    {
        std::string loc_value = std::format("{{name: \"{}\",value: \"{}\", style: \"{}\"}}",a_name,a_value,a_style);
        a_input += loc_value;
        if (a_sep) a_input += ",";
    }

}