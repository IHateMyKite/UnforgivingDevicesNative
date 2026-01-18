#include <UD_ModuleManager.h>

#include <UD_Config.h>
#include <UD_PapyrusDelegate.h>

SINGLETONBODY(UD::ModuleManager)

void UD::ModuleManager::Update(float a_delta)
{
    //DEBUG("Update({},{},{}) called",a_delta,_WaitingForPapyrus,_Delay)
    if (!_WaitingForPapyrus)
    {
        _Delay -= a_delta;
        if (_Delay <= 0.0) 
        {
            _Delay = 0.0;
            UpdateModuleVariables();
            if (AllModulesReady())
            {
                if (!_SetupMessagePrinted)
                {
                    _SetupMessagePrinted = true;
                    CLOG("[Unforgiving Devices] INITIATED! - {} modules Ready",_modules.size())
                }
                const int loc_res = CallReload();
                if ((loc_res == 2) && !_ReloadMessagePrinted)
                {
                    CLOG("[Unforgiving Devices] RELOADED! - {} modules Reloaded",_modules.size())
                    _ReloadMessagePrinted = true;
                }
                else if (loc_res == 0)
                {
                    _MessageTimer -= a_delta;
                    if (_MessageTimer <= 0.0f) 
                    {
                        CLOG("[Unforgiving Devices] Reloading in progress.....")
                        _MessageTimer = 1.0;
                    }
                }
            }
            else
            {
                _SetupMessagePrinted = false;
                const int loc_res = CallSetup();
                if (loc_res == 0)
                {
                    _MessageTimer -= a_delta;
                    if (_MessageTimer <= 0.0f) 
                    {
                        CLOG("[Unforgiving Devices] Initialization in progress.....")
                        _MessageTimer = 1.0;
                    }
                }
            }
        }
    }
}

void UD::ModuleManager::Reload(bool a_setDelay)
{
    DEBUG("Reload called. Set delay ? : {}", a_setDelay)
    Clean();
    _WaitForReload = true;
    if (a_setDelay) SetDelay(2.0); // Wait for one sec
}

void UD::ModuleManager::Clean()
{
    DEBUG("Clean called")
    _WaitingForPapyrus = true;
    _modules.clear();
}

void UD::ModuleManager::SetPapyrusReady()
{
    DEBUG("SetPapyrusReady called")
    _WaitingForPapyrus = false;
    UpdateModuleVariables();
    if (_WaitForReload)
    {
        ResetReloaded();
        _WaitForReload = false;
    }
}

void UD::ModuleManager::SetDelay(float a_time)
{
    _Delay = a_time;
}

bool UD::ModuleManager::IsReady(bool a_CheckReload)
{
    return !_WaitingForPapyrus && AllModulesReady() && (AllModulesReloaded() || !a_CheckReload);
}

RE::TESQuest* UD::ModuleManager::GetModuleByAlias(std::string a_alias)
{
    if (_WaitingForPapyrus)
    {
        return nullptr;
    }
    std::transform(a_alias.begin(), a_alias.end(), a_alias.begin(), ::tolower);

    for (auto&& [handle,module] : _modules)
    {
        std::string loc_alias = module.Alias;
        std::transform(loc_alias.begin(), loc_alias.end(), loc_alias.begin(), ::tolower);

        if (loc_alias == a_alias)
        {
            return module.quest;
        }
    }
    return nullptr;
}

std::vector<RE::TESQuest*> UD::ModuleManager::GetModules()
{
    if (_WaitingForPapyrus)
    {
        return std::vector<RE::TESQuest*>();
    }

    const std::vector<Module*> loc_modulesSorted = GetSortedModuleList();

    std::vector<RE::TESQuest*> loc_res;

    std::for_each(loc_modulesSorted.begin(),loc_modulesSorted.end(),[&](Module* a_m){loc_res.push_back(a_m->quest);});

    return loc_res;
}

std::vector<RE::TESQuest*> UD::ModuleManager::GetModuleDependency(RE::TESQuest* a_module)
{
    if (_WaitingForPapyrus)
    {
        return std::vector<RE::TESQuest*>();
    }

    Module* loc_module = GetModuleByQuest(a_module);
    return loc_module->Dependency;
}

std::vector<RE::TESQuest*> UD::ModuleManager::GetDependantModules(RE::TESQuest* a_module)
{
    if (_WaitingForPapyrus)
    {
        return std::vector<RE::TESQuest*>();
    }

    //Module* loc_module = GetModuleByQuest(a_module);
    std::vector<RE::TESQuest*> loc_res;
    const std::vector<Module*> loc_modulesSorted = GetSortedModuleList();
    std::for_each(loc_modulesSorted.begin(),loc_modulesSorted.end(),[&](Module* a_m)
    {
        if (std::find(a_m->Dependency.begin(),a_m->Dependency.end(),a_module) == a_m->Dependency.end()) loc_res.push_back(a_m->quest);
    });

    return loc_res;
}

void UD::ModuleManager::ResetModule(RE::TESQuest* a_module)
{
    Module* loc_module = GetModuleByQuest(a_module);
    loc_module->quest->ResetAndUpdate();
}

void UD::ModuleManager::ResetAllModules()
{
    auto loc_modules = GetSortedModuleList();
    for (auto&& module : loc_modules)
    {
        module->quest->ResetAndUpdate();
    }
    _Delay = 1.0;
}

std::vector<RE::TESQuest*> UD::ModuleManager::GetModulesByScript(std::string a_script)
{
    if (_WaitingForPapyrus)
    {
        return std::vector<RE::TESQuest*>();
    }
    std::vector<RE::TESQuest*> loc_res;

    for (auto&& [handle,module] : _modules)
    {
        if (module.object.get() && PapyrusDelegate::GetSingleton()->HaveScriptBase(module.object->GetTypeInfo(),a_script))
        {
            loc_res.push_back(module.quest);
        }
    }
    return loc_res;
}

std::vector<RE::BGSBaseAlias*> UD::ModuleManager::GetModulesAliasesByScript(std::string a_script)
{
    if (_WaitingForPapyrus)
    {
        return std::vector<RE::BGSBaseAlias*>();
    }
    std::vector<RE::BGSBaseAlias*> loc_res;

    std::vector<RE::TESQuest*> loc_modules = GetModulesByScript(a_script);

    for (auto&& it : loc_modules)
    {
        loc_res.insert(loc_res.end(),it->aliases.begin(),it->aliases.end());
    }

    return loc_res;
}

void UD::ModuleManager::AddModule(RE::VMHandle a_handle, Module a_module)
{
    DEBUG("AddModule called")
    _modules[a_handle] = a_module;
}

void UD::ModuleManager::UpdateModuleVariables()
{
    //DEBUG("UpdateModuleVariables called")
    for (auto&& [handle,module] : _modules)
    {
        auto loc_object     = module.object;
        //module.Name         = (loc_object->GetProperty("MODULE_NAME")   != nullptr) ? loc_object->GetProperty("MODULE_NAME")->GetString()  : "NONAME";
        module.Name         = module.quest->GetName();
        module.Priority     = (loc_object->GetProperty("MODULE_PRIO")   != nullptr) ? loc_object->GetProperty("MODULE_PRIO")->GetUInt()  : 0x0U;
        module.Alias        = (loc_object->GetProperty("MODULE_ALIAS")  != nullptr) ? loc_object->GetProperty("MODULE_ALIAS")->GetString()  : "";
        module.Description  = (loc_object->GetProperty("MODULE_DESC")   != nullptr) ? loc_object->GetProperty("MODULE_DESC")->GetString()  : "";
        module.SetupCalled  = (loc_object->GetVariable("_SetupCalled")  != nullptr) ? loc_object->GetVariable("_SetupCalled")->GetBool() : false;
        module.SetupDone    = (loc_object->GetVariable("_SetupDone")    != nullptr) ? loc_object->GetVariable("_SetupDone")->GetBool()   : false;
        module.ReloadCalled = (loc_object->GetVariable("_ReloadCalled") != nullptr) ? loc_object->GetVariable("_ReloadCalled")->GetBool()  : false;
        module.ReloadDone   = (loc_object->GetVariable("_ReloadDone")   != nullptr) ? loc_object->GetVariable("_ReloadDone")->GetBool()  : false;

        module.Dependency.clear();
        if (loc_object->GetVariable("MODULE_DEP") && loc_object->GetVariable("MODULE_DEP")->GetArray())
        {
            auto loc_array = loc_object->GetVariable("MODULE_DEP")->GetArray();
            for (auto&& it : *loc_array)
            {
                RE::TESQuest* loc_quest = static_cast<RE::TESQuest*>(it.GetObject()->Resolve((RE::VMTypeID)RE::FormType::Quest));
                module.Dependency.push_back(loc_quest);
            }
        }
    }
 }

 // https://github.com/powerof3/SKSEPlugins/blob/2d55e7f0966bfd23c330ef05a692213965554eda/Skyrim/include/Skyrim/BSScript/BSScriptIForEachScriptObjectFunctor.h#L10
class IForEachScriptObjectFunctor
{
public:
	enum ResultType
	{
		kResult_Abort = 0,
		kResult_Continue = 1,
	};

	IForEachScriptObjectFunctor() {};
	virtual ~IForEachScriptObjectFunctor() {};

	// return true to continue
	virtual uint32_t Visit(RE::BSScript::Object * object, bool bConditional) = 0;
};

int UD::ModuleManager::CallSetup()
{
    if (AllModulesReady())
    {
        return 2;
    }

    const auto loc_vm = InternalVM::GetSingleton();

    std::vector<Module*> loc_modulesSorted = GetSortedModuleList();


    //DEBUG("Checking {} module(s) for setup",_modulesSorted.size())

    for (auto&& module : loc_modulesSorted)
    {
        //DEBUG("{} = 0x{:016X}",module->Alias,(uintptr_t)module)
        auto loc_object = module->object;
 
        if (module->SetupCalled == false)
        {
            // Check dependency first
            bool loc_dependency = true;
            for (auto&& it : module->Dependency)
            {
                if (!IsQuestReady(it))
                {
                    loc_dependency = false;
                    break;
                }
            }
            
            if (loc_dependency)
            {
                if (module->quest->IsRunning() && (module->quest->data.flags.underlying() & 0x0001U))
                {
                    DEBUG("Quest for module {} running. Calling papyrus _Setup function",module->Alias)

                    //init unused callback
                    RE::BSTSmartPointer<RE::BSScript::IStackCallbackFunctor> loc_callback;
                    
                    auto loc_arg = RE::MakeFunctionArguments();
                    
                    //call papyrus method
                    loc_vm->DispatchMethodCall(loc_object,"_Setup",loc_arg,loc_callback);
                    
                    delete loc_arg;
                    
                    return 0; // Only one operation per update to prevent VM from breaking
                }
                else if (module->QuestStarting == false)
                {
                    DEBUG("Quest for module {} not running, starting quest",module->Alias)
                    // Try to start quest
                    //module->quest->ResetAndUpdate();
                    module->quest->Stop();
                    module->quest->Start();
                    module->QuestStarting = true;
                    return 0; // Only one operation per update to prevent VM from breaking
                }
                else
                {
                    // Quest is starting, wait
                    DEBUG("Quest for module {} not running. Waiting for it to start",module->Alias)
                    return 0; // Only one operation per update to prevent VM from breaking
                }
            }
            else
            {
                DEBUG("Dependency of module {} not fullfilled yet, skipping",module->Alias)
            }
        }
        else if (module->SetupDone == true)
        {
            // Module is ready
            //DEBUG("Module {} is ready",module->Alias)
        }
        else
        {
            // Waiting for module to finish setup
            DEBUG("Waiting for Module {} to finish its setup",module->Alias)
            return 0;
        }
    }
    return 2;
}

int UD::ModuleManager::CallReload()
{
    if (AllModulesReloaded())
    {
        //DEBUG("All modules already reloaded, skipping")
        return 2;
    }

    const auto loc_vm = InternalVM::GetSingleton();

    std::vector<Module*> loc_modulesSorted = GetSortedModuleList();
    //DEBUG("Checking {} module(s) for reload",_modulesSorted.size())

    for (auto&& module : loc_modulesSorted)
    {
        auto loc_object = module->object;
        //DEBUG("Checking if module {} is running",module->Alias)
        if (!module->ReloadCalled)
        {
            DEBUG("Flag 0x{:04X}",module->quest->data.flags.underlying())
            auto loc_setupcalled = loc_object->GetVariable("_ReloadCalled");
            if (loc_setupcalled) loc_setupcalled->SetBool(true);

            DEBUG("Module {} ready. Calling papyrus _GameReload function",module->Alias)

            //init unused callback
            RE::BSTSmartPointer<RE::BSScript::IStackCallbackFunctor> loc_callback;
            
            auto loc_arg = RE::MakeFunctionArguments();
    
            //call papyrus method
            loc_vm->DispatchMethodCall(loc_object,"_GameReload",loc_arg,loc_callback);
    
            delete loc_arg;
            return 0;
        }
        else if (!module->ReloadDone)
        {
            // Waiting for reload to finish
            return 0;
        }
        else
        {
            // Reload finished
        }
    }
    return 2;
}

std::vector<UD::Module*> UD::ModuleManager::GetSortedModuleList()
{
    std::vector<Module*> loc_modulesSorted;
    for (auto&& [handle,module] : _modules)
    {
        loc_modulesSorted.push_back(&module);
    }

    // Order modules by priority
    std::sort(loc_modulesSorted.begin(),loc_modulesSorted.end(),[&](const Module * const a_m1,const Module * const a_m2) -> bool
    {
        return a_m1->Priority > a_m2->Priority;
    });
    return loc_modulesSorted;
}

UD::Module* UD::ModuleManager::GetModuleByQuest(RE::TESQuest* a_quest)
{
    for (auto&& [handle,module] : _modules)
    {
        if (module.quest == a_quest) return &module;
    }
    return nullptr;
}

bool UD::ModuleManager::IsQuestReady(RE::TESQuest* a_quest, bool a_checkreload)
{
    if (a_quest == nullptr) return false;
    bool loc_ready = false;
    Module* loc_module = GetModuleByQuest(a_quest);
    if (loc_module)
    {
        auto loc_setupdonevar = loc_module->object->GetVariable("_SetupDone");
        if (loc_setupdonevar) loc_ready = loc_setupdonevar->GetBool();
        else loc_ready = true; // If quest doesnt have variable, just ignore it
    }
    else
    {
        loc_ready = true; // If quest doesnt have script, just ignore it
    }

    return loc_ready && a_quest->IsRunning() && (a_quest->data.flags.underlying() & 0x0001U);
}

bool UD::ModuleManager::AllModulesReady()
{
    bool loc_res = true;
    for (auto&& [handle,module] : _modules)
    {
        loc_res = loc_res && module.SetupDone && module.quest->IsRunning() && (module.quest->data.flags.underlying() & 0x0001U);
    }
    //DEBUG("AllModulesReady() -> {}",loc_res)
    return loc_res;
}

bool UD::ModuleManager::AllModulesReloaded()
{
    bool loc_res = true;
    for (auto&& [handle,module] : _modules)
    {
        loc_res = loc_res && module.ReloadDone;
    }
    //DEBUG("AllModulesReloaded() -> {}",loc_res)
    return loc_res;
}

void UD::ModuleManager::ResetReloaded()
{
    _ReloadMessagePrinted = false;
    for (auto&& [handle,module] : _modules)
    {
        auto loc_object = module.object;
        //DEBUG("Checking if module {} is running",module->Alias)
        if (module.ReloadDone == true)
        {
            auto loc_reloadcalled = loc_object->GetVariable("_ReloadCalled");
            if (loc_reloadcalled) loc_reloadcalled->SetBool(false);

            auto loc_reloaddone = loc_object->GetVariable("_ReloadDone");
            if (loc_reloaddone) loc_reloaddone->SetBool(false);
            DEBUG("Resetting reloaded module {}({})",module.Name,module.Alias)
        }
    }
}
