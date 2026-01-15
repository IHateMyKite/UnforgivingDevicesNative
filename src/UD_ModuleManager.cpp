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
                CallReload();
            }
            else
            {
                CallSetup();
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
    //loc_module->SetupDone       = false;
    //loc_module->SetupCalled     = false;
    //loc_module->ReloadDone      = false;
    //loc_module->ReloadCalled    = false;
    //loc_module->QuestStarting   = false;
    //loc_module->quest->Stop();
    loc_module->quest->ResetAndUpdate();
}

void UD::ModuleManager::AddModule(RE::VMHandle a_handle, Module a_module)
{
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

void UD::ModuleManager::CallSetup()
{
    if (AllModulesReady())
    {
        return;
    }

    const auto loc_vm = InternalVM::GetSingleton();

    std::vector<Module*> loc_modulesSorted = GetSortedModuleList();

    //DEBUG("Checking {} module(s) for setup",_modulesSorted.size())

    for (auto&& module : loc_modulesSorted)
    {
        auto loc_object = module->object;
        //DEBUG("Checking if module {} is running",module->Alias)
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
                    auto loc_setupcalled = loc_object->GetVariable("_SetupCalled");
                    if (loc_setupcalled) loc_setupcalled->SetBool(true);
                
                    DEBUG("Quest for module {} running. Calling papyrus _Setup function",module->Alias)

                    //init unused callback
                    RE::BSTSmartPointer<RE::BSScript::IStackCallbackFunctor> loc_callback;
            
                    auto loc_arg = RE::MakeFunctionArguments();
    
                    //call papyrus method
                    loc_vm->DispatchMethodCall(loc_object,"_Setup",loc_arg,loc_callback);
    
                    delete loc_arg;
                    return; // Only one operation per update to prevent VM from breaking
                }
                else if (module->QuestStarting == false)
                {
                    DEBUG("Quest for module {} not running, starting quest",module->Alias)
                    // Try to start quest
                    module->quest->Start();
                    module->QuestStarting = true;
                    return; // Only one operation per update to prevent VM from breaking
                }
                else
                {
                    // Quest is starting, wait
                    DEBUG("Quest for module {} not running. Waiting for it to start",module->Alias)
                    return; // Only one operation per update to prevent VM from breaking
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
        }
    }
}

void UD::ModuleManager::CallReload()
{
    if (AllModulesReloaded())
    {
        //DEBUG("All modules already reloaded, skipping")
        return;
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
            auto loc_setupcalled = loc_object->GetVariable("_ReloadCalled");
            if (loc_setupcalled) loc_setupcalled->SetBool(true);

            DEBUG("Module {} ready. Calling papyrus _GameReload function",module->Alias)

            //init unused callback
            RE::BSTSmartPointer<RE::BSScript::IStackCallbackFunctor> loc_callback;
            
            auto loc_arg = RE::MakeFunctionArguments();
    
            //call papyrus method
            loc_vm->DispatchMethodCall(loc_object,"_GameReload",loc_arg,loc_callback);
    
            delete loc_arg;
            return;
        }
        else if (!module->ReloadDone)
        {
            // Waiting for reload to finish
            return;
        }
        else
        {
            // Reload finished
        }
    }
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
            DEBUG("Ressetting reloaded module {}({})",module.Name,module.Alias)
        }
    }
}
