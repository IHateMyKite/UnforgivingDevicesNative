#include <UD_ModuleManager.h>

#include <UD_Config.h>
#include <UD_PapyrusDelegate.h>

SINGLETONBODY(UD::ModuleManager)

void UD::ModuleManager::Setup()
{
    if (!_installed)
    {
        _installed = true;
    }
}

void UD::ModuleManager::Update()
{
    UpdateModuleVariables();
    CallSetupOrUpdate();
}

void UD::ModuleManager::AddModule(RE::VMHandle a_handle, Module a_module)
{
    _modules[a_handle] = a_module;
}

void UD::ModuleManager::UpdateModuleVariables()
{
    //for (auto&& [handle,module] : _modules)
    //{
    //    auto loc_object     = module.object;
    //    module.PRIORITY     = (loc_object->GetProperty("PRIORITY")     != nullptr) ? loc_object->GetProperty("PRIORITY")->GetUInt()     : 0xFFFFFFFFU;
    //    module._SetupCalled = (loc_object->GetVariable("_SetupCalled") != nullptr) ? loc_object->GetVariable("_SetupCalled")->GetBool() : false;
    //    module._SetupDone   = (loc_object->GetVariable("_SetupDone")   != nullptr) ? loc_object->GetVariable("_SetupDone")->GetBool()   : false;
    //    module._Updating    = (loc_object->GetVariable("_Updating")    != nullptr) ? loc_object->GetVariable("_Updating")->GetBool()    : false;
    //    module._Disabled    = (loc_object->GetVariable("_Disabled")    != nullptr) ? loc_object->GetVariable("_Disabled")->GetBool()    : false;
    //}
}

void UD::ModuleManager::CallSetupOrUpdate()
{
    //const auto loc_vm = InternalVM::GetSingleton();
    //for (auto&& [handle,module] : _modules)
    //{
    //    auto loc_object = module.object;
    //    if ((module._SetupCalled == false) && (module._SetupDone == false) && (module._Disabled == false))
    //    {
    //        auto loc_setupcalled = loc_object->GetVariable("SetupCalled");
    //        if (loc_setupcalled) loc_setupcalled->SetBool(true);
    //        
    //        //init unused callback
    //        RE::BSTSmartPointer<RE::BSScript::IStackCallbackFunctor> loc_callback;
    //        
    //        auto loc_arg = RE::MakeFunctionArguments();
    //
    //        //call papyrus method
    //        loc_vm->DispatchMethodCall(loc_object,"Setup",loc_arg,loc_callback);
    //
    //        delete loc_arg;
    //        return; // Only one operation per update to prevent VM from breaking
    //    }
    //}
}
