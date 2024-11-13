#include <UD_PapyrusDelegate.h>

#include <execution>

#include <windows.h>
#include <UD_Macros.h>  // needs to be included again because windows.h adds its own ERROR macro

#include <UD_Config.h>
#include <UD_Utility.h>
//#include <UD_Spinlock.h>
#include <UD_DDAPI.h>

#undef GetObject

SINGLETONBODY(UD::PapyrusDelegate)

using UD::PapyrusDelegate;
typedef PapyrusDelegate::Result Result;
typedef PapyrusDelegate::FilterDeviceResult FilterDeviceResult;

RE::VMHandle UD::PapyrusDelegate::ToVMHandle(const int a_1, const int a_2)
{
    const uint64_t loc_fir =  (*(uint32_t*)&a_1);
    const uint64_t loc_sec = ((uint64_t)(*(uint32_t*)&a_2) << 32);
    return loc_fir | loc_sec;
}

void PapyrusDelegate::Setup()
{
    Utils::UniqueLock lock(_SaveLock);
    if (!_installed)
    {
        _udrdkw = reinterpret_cast<RE::BGSKeyword*>(RE::TESDataHandler::GetSingleton()->LookupForm(0x11A352,"UnforgivingDevices.esp"));
        _installed = true;
        LOG("PapyrusDelegate::Setup - installed")
    }

    // update static scripts
    const auto loc_vm = InternalVM::GetSingleton();
    for (auto&& [form,formvm] : _modscripts)
    {
        auto loc_form = RE::TESDataHandler::GetSingleton()->LookupForm(form.formid,form.modname);
        if (loc_form)
        {
            formvm.handle = loc_vm->GetObjectHandlePolicy()->GetHandleForObject(loc_form->GetFormType(),loc_form);
            loc_vm->FindBoundObject(formvm.handle, form.scriptname.c_str(), formvm.object);

            if (formvm.handle && formvm.object) DEBUG("Script {} loaded",form.scriptname)
            else ERROR("Could not load script {}",form.scriptname)
        }
        else ERROR("Cant find form {}:{}",form.formid,form.modname)
    }
}

void UD::PapyrusDelegate::Reload()
{
    SKSE::GetTaskInterface()->AddTask([this]
    {
        Setup();
        ResetCache();

        Utils::UniqueLock lock(_SaveLock);
        UpdateVMHandles();
    });
}

int PapyrusDelegate::RegisterDeviceScripts(RE::Actor* a_actor)
{
    LOG("RegisterDeviceScripts({}) - Called",a_actor ? a_actor->GetName() : "NONE")
    if (a_actor == nullptr) return 0;

    auto loc_slot = std::find_if(_npcslotcache.begin(),_npcslotcache.end(),[&](std::pair<const RE::VMHandle,NPCSlot> a_it)
    {
        if (a_it.second.alias && a_it.second.alias->GetActorReference() == a_actor)
        {
            return true;
        }
        return false;
    });

    if (loc_slot == _npcslotcache.end())
    {
        ERROR("RegisterDeviceScripts({}) - Cant register devices because actor is not registered in any of the slots",a_actor ? a_actor->GetName() : "NONE")
        return 0;
    }

    auto loc_devices = DeviousDevicesAPI::g_API->GetWornDevices(a_actor);
    if (loc_devices.size() == 0) return 0;

    auto loc_devicescripts = FindAllDeviceScripts(a_actor);
    if (loc_devicescripts.size() == 0) return 0;

    auto loc_slotobject = loc_slot->second.object;
    auto loc_arrobject = loc_slotobject ? loc_slotobject->GetProperty("UD_equipedCustomDevices") : nullptr;
    auto loc_arr = loc_arrobject ? loc_arrobject->GetArray() : nullptr;

    if (!loc_arrobject || !loc_arr)
    {
        ERROR("RegisterDeviceScripts({}) - Cant register devices because slot doesnt have device slots ready",a_actor ? a_actor->GetName() : "NONE")
        return 0;
    }

    // Fuck you and your private bullshit and assert non sense
    struct Variable_
    {
        RE::BSScript::TypeInfo  varType;  // 00
        Variable::Value         value;    // 08
    };

    // Reset array
    for (auto&& it : *loc_arr)
    {
        (*(Variable_*)(&it)).value.obj.~BSTSmartPointer();
    }

    int loc_registered = 0;
    for (auto&& object : loc_devicescripts)
    {
        auto loc_rd = GetScriptVariable<RE::TESObjectARMO>(object,"_DeviceRendered",RE::FormType::Armor);
        if (loc_rd && (std::find(loc_devices.begin(),loc_devices.end(),loc_rd) != loc_devices.end()))
        {
            auto loc_emptyslot = std::find_if(loc_arr->begin(),loc_arr->end(),[](Variable& a_var)
            {
                if (a_var.GetObject()) return false;
                else return true;
            });

            if (loc_emptyslot)
            {
                loc_emptyslot->SetObject(object,loc_emptyslot->GetType().GetRawType());
                loc_registered++;
                continue;
            }
        }
    }

    return loc_registered;
}

Result PapyrusDelegate::SendMinigameThreadEvents(RE::Actor* a_actor, RE::TESObjectARMO* a_device, RE::VMHandle a_handle,MinigameThreads a_threads)
{
    LOG("SendMinigameThreadEvents({},{:08X},{:016X},{}) called",a_actor ? a_actor->GetName() : "NONE",a_device ? a_device->GetFormID() : 0x0,a_handle,a_threads)
    if (a_actor == nullptr || a_device == nullptr || a_threads == 0) return Result::rArgError;
    if (!a_device->HasKeyword(_udrdkw)) return Result::rDeviceError;

    std::vector<RE::TESObjectARMO*> loc_device = {a_device};

    RE::VMHandle loc_vmhandle = a_handle;
    loc_vmhandle = ValidateVMHandle(loc_vmhandle,a_device);

    const auto loc_vm = InternalVM::GetSingleton();

    auto loc_cacheres = _cache[loc_vmhandle];
    if (loc_cacheres.object != nullptr)
    {
        LOG("Device object found in cache - using it")

        //init unused callback
        RE::BSTSmartPointer<RE::BSScript::IStackCallbackFunctor> loc_callback1;
        RE::BSTSmartPointer<RE::BSScript::IStackCallbackFunctor> loc_callback2;
        RE::BSTSmartPointer<RE::BSScript::IStackCallbackFunctor> loc_callback3;
        RE::BSTSmartPointer<RE::BSScript::IStackCallbackFunctor> loc_callback4;
        auto loc_arg = RE::MakeFunctionArguments();
        //call papyrus method
        if (a_threads & tStarter)   loc_vm->DispatchMethodCall(loc_cacheres.object,"_MinigameStarterThread",loc_arg,loc_callback1);
        if (a_threads & tCrit)      loc_vm->DispatchMethodCall(loc_cacheres.object,"_MinigameCritLoopThread",loc_arg,loc_callback2);
        if (a_threads & tParalel)   loc_vm->DispatchMethodCall(loc_cacheres.object,"_MinigameParalelThread",loc_arg,loc_callback3);
        if (a_threads & tAV)        loc_vm->DispatchMethodCall(loc_cacheres.object,"_MinigameAVCheckLoopThread",loc_arg,loc_callback4);
        delete loc_arg;

        LOG("PapyrusDelegate::SendMinigameThreadEvents - events sent")

        return Result::rSuccess;
    }
    return Result::rNotFound;
}

Result PapyrusDelegate::SendRemoveRenderDeviceEvent(RE::Actor* a_actor, RE::TESObjectARMO* a_device)
{
    LOG("SendRemoveRenderDeviceEvent({},0x{:08X}) called",a_actor ? a_actor->GetName() : "NONE",a_device ? a_device->GetFormID() : 0x0)
    if (a_actor == nullptr || a_device == nullptr) return Result::rArgError;
    if (!a_device->HasKeyword(_udrdkw)) return Result::rDeviceError;

    const auto loc_vm = InternalVM::GetSingleton();

    ValidateCache();

    for(auto&& [vmhandle,cached] : _cache)
    {
        if (cached.rd == a_device)
        {
            LOG("Device object found in cache - using it")

            auto loc_device = cached.object;
            auto isRemovedProperty=loc_device->GetProperty("_isRemoved");
            if (isRemovedProperty==nullptr)
            {
                continue;
            }
            const bool loc_isremoved = (isRemovedProperty->GetBool());

            if (loc_isremoved) continue; //every device can be only removed once

            const RE::Actor* loc_wearer = GetScriptVariable<RE::Actor>(loc_device,"Wearer",RE::FormType::ActorCharacter);

            if (loc_wearer == a_actor)
            {
                const RE::VMHandle loc_newhandle = (static_cast<RE::VMHandle>(_RemovedCounter++ << 48U) | 0x0000FFFFFFFFFFFFUL);

                //set new unique VMHandle
                const auto loc_var1 = loc_device->GetVariable("_VMHandle1");
                const auto loc_var2 = loc_device->GetVariable("_VMHandle2");
                loc_var1->SetUInt(loc_newhandle & 0xFFFFFFFF);
                loc_var2->SetUInt((loc_newhandle >> 32) & 0xFFFFFFFF);

                LOG("New VMHandle = 0x{:016X}",loc_newhandle)

                _cache[loc_newhandle] = cached;
                _cache.erase(vmhandle);

                //init unused callback
                RE::BSTSmartPointer<RE::BSScript::IStackCallbackFunctor> loc_callback;
                auto loc_args = new RE::BSScript::FunctionArguments<void, RE::Actor*>(std::forward<RE::Actor*>(a_actor));
                //call papyrus method
                loc_vm->DispatchMethodCall(loc_device,"removeDevice",loc_args,loc_callback);
                delete loc_args;

                LOG("SendRemoveRenderDeviceEvent({},0x{:08X}) - event sent",a_actor->GetName(),a_device->GetFormID())

                return Result::rSuccess;
            }
        }
    }
    return Result::rNotFound;
}

Result UD::PapyrusDelegate::SetBitMapData(RE::VMHandle a_handle, RE::TESObjectARMO* a_device, std::string a_name, int a_val, uint8_t a_size, uint8_t a_off)
{
    LOG("SetBitMapData(0x{:016X},0x{:08X},{},{},{},{}) called",a_handle,a_device ? a_device->GetFormID() : 0x0 ,a_name,a_val,a_size,a_off)
    if (a_name == "" || a_size > 32 || a_off > 32 || a_device == nullptr) return Result::rArgError;
    if (!a_device->HasKeyword(_udrdkw)) return Result::rDeviceError;

    RE::VMHandle loc_vmhandle = a_handle;
    loc_vmhandle = ValidateVMHandle(loc_vmhandle,a_device);

    ValidateCache();
    auto loc_cacheres = _cache[loc_vmhandle];

    if (loc_cacheres.object != nullptr)
    {
        RE::Actor* loc_wearer = RE::Actor::LookupByHandle(loc_cacheres.wearer).get();
        LOG("Device object found in cache - using it - Wearer = {}",loc_wearer ? loc_wearer->GetName() : "NONE")

        const auto loc_var = loc_cacheres.object->GetVariable(a_name);

        if (loc_var == nullptr)
        {
            ERROR("Could not find bitmap variable {}",a_name)
            return Result::rNotFound;
        }
        int loc_res = Utility::GetSingleton()->CodeBit(loc_var->GetSInt(),a_val,a_size,a_off);
        loc_var->SetSInt(loc_res);

        return Result::rSuccess;
    }
    return Result::rNotFound;
}

RE::VMHandle UD::PapyrusDelegate::ValidateVMHandle(RE::VMHandle a_handle, RE::TESObjectARMO* a_device)
{
    LOG("PapyrusDelegate::ValidateVMHandle(0x{:016X},0x{:08X}) - called",a_handle,a_device ? a_device->GetFormID() : 0U)

    if (a_device == nullptr) return a_handle;

    if (a_handle != 0) return a_handle;

    const auto loc_vm = InternalVM::GetSingleton();

    RE::VMHandle loc_res = a_handle;
    loc_vm->attachedScriptsLock.Lock();
    for (auto&& it : loc_vm->attachedScripts)
    {
        auto loc_type = HaveScriptBase(it.second,"ud_customdevice_renderscript");
        if (loc_type != nullptr)
        {
            FilterDeviceResult loc_filterres = ProcessDevice2(it.first,0,loc_type,a_device,[&](RE::BSTSmartPointer<RE::BSScript::Object> a_object,RE::TESObjectARMO* a_id,RE::TESObjectARMO* a_rd) -> bool
            {
                if (a_object == nullptr) return false;

                const auto loc_var1 = a_object->GetVariable("_VMHandle1");
                const auto loc_var2 = a_object->GetVariable("_VMHandle2");

                if (loc_var1 == nullptr || loc_var2 == nullptr) return false;

                _cache[it.first].object  = a_object;
                _cache[it.first].id      = a_id;
                _cache[it.first].rd      = a_rd;
                auto loc_wearer = GetScriptVariable<RE::Actor>(a_object,"Wearer",RE::FormType::ActorCharacter);
                _cache[it.first].wearer  = loc_wearer ? loc_wearer->GetHandle().native_handle() : 0x0U;
                

                if (loc_var1->GetUInt() == 0 && loc_var2->GetUInt() == 0) 
                {
                    loc_var1->SetUInt(it.first & 0xFFFFFFFF);
                    loc_var2->SetUInt((it.first >> 32) & 0xFFFFFFFF);
                    LOG("Handle of {} set to 0x{:016X} = 0x{:08X} + 0x{:08X} ; Wearer = {}",a_id ? a_id->GetName() : "None",it.first,loc_var1->GetUInt(),loc_var2->GetUInt(),loc_wearer ? loc_wearer->GetName() : "NONE")
                    loc_res = it.first;
                    return true;
                }
                return false;
            });
            if (loc_filterres.Result) break;
        }
    }
    loc_vm->attachedScriptsLock.Unlock();

    if (loc_res == 0)
    {
        LOG("Could not find script for device 0x{:08X}",a_device->GetFormID())
    }

    return loc_res;
}

void UD::PapyrusDelegate::ValidateCache() const
{
    LOG("PapyrusDelegate::ValidateCache() - called")

    std::vector<RE::VMHandle> loc_toremove;

    for (auto&& [vmhandle,cached] : _cache)
    {
        if (cached.object.get() == nullptr)
        {
            ERROR("Device {} have null script object",cached.id ? cached.id->GetName() : "NONE")
            loc_toremove.push_back(vmhandle);
            continue;
        }

        //sometimes the device can be  cached before wearer is set, so we check it now
        auto loc_wearer = RE::Actor::LookupByHandle(cached.wearer).get();
        if (loc_wearer == nullptr)
        {
            loc_wearer = GetScriptVariable<RE::Actor>(cached.object,"Wearer",RE::FormType::ActorCharacter);
            cached.wearer = loc_wearer ? loc_wearer->GetHandle().native_handle() : 0x0U;
        }

        if (!cached.rd && cached.id)
        {
            auto loc_rd = DeviousDevicesAPI::g_API->GetDeviceRender(cached.id);
            cached.rd = loc_rd;

            LOG("Getting render device from DD database => 0x{:08X}",loc_rd ? loc_rd->GetFormID() : 0)

            auto loc_objvar = cached.object->GetVariable("_DeviceRendered");
            RE::BSScript::PackHandle(loc_objvar,loc_rd,(RE::VMTypeID)RE::FormType::Armor);
        }

        if (cached.object->refCountAndHandleLock > 2) continue; //if number of references is 2, it means that no thread is currently running on the object (hopefully)

        auto loc_var = cached.object->GetProperty("_isRemoved");
        if (loc_var == nullptr) continue;

        bool loc_val = loc_var->GetBool();
        if (loc_val)
        {
            loc_toremove.push_back(vmhandle);
            LOG("PapyrusDelegate::ValidateCache - {} is no longer valid - Removing from cache",cached.id->GetName())
        }
    }

    for(auto&& it : loc_toremove) _cache.erase(it);

    ValidateInvalidDevices();
}

void UD::PapyrusDelegate::ValidateInvalidDevices() const
{
    const auto loc_vm = InternalVM::GetSingleton();
    std::for_each(std::execution::seq,loc_vm->objectsAwaitingCleanup.begin(),loc_vm->objectsAwaitingCleanup.end(),[&](RE::BSTSmartPointer<RE::BSScript::Object>& a_script)
    {
        auto loc_removed = std::find_if(std::execution::seq,_removeddevices.begin(),_removeddevices.end(),[a_script](Device& a_device)
        {
            return a_device.object.get() == a_script.get();
        });

        if (loc_removed != _removeddevices.end())
        {
            return;
        }

        auto loc_type = HaveScriptBase(a_script->GetTypeInfo(),"ud_customdevice_renderscript");
        if (loc_type != nullptr)
        {
            static size_t loc_cnt = 0;
            RE::VMHandle loc_handle = a_script->handle;
            //get script object
            RE::BSTSmartPointer<RE::BSScript::Object> loc_object = a_script;
        
            if (loc_object != nullptr)
            {
                //undef this stupidass macro so we can use the GetObject method
                #undef GetObject
        
                //get inventory device from papyrus script
                auto loc_id = GetScriptProperty<RE::TESObjectARMO>(loc_object,"DeviceInventory",RE::FormType::Armor);
                if (loc_id != nullptr)
                {
                    // device is currently getting unlocked. Because of that, the device should be valid
                    auto loc_unlocked = loc_object->GetVariable("_IsUnlocked");
                    if (loc_unlocked && (loc_unlocked->GetBool() == true))
                    {
                        DEBUG("ValidateInvalidDevices - Device 0x{:016X} is unlocked. Skipping",loc_handle)
                        return;
                    }

                    const auto loc_var1 = loc_object->GetVariable("_VMHandle1");
                    const auto loc_var2 = loc_object->GetVariable("_VMHandle2");
        
                    if (loc_var1 == nullptr || loc_var2 == nullptr) return;
        
                    loc_handle = ((uint64_t)loc_var1->GetUInt() | ((uint64_t)loc_var2->GetUInt() << 32U));

                    if (loc_handle == 0x0000FFFF00000000)
                    {
                        
                        static uint64_t loc_counter = 0;
                        loc_handle |= (loc_counter++);
                    }

                    // Check if device is already not removed
                    auto loc_unregistered = loc_object->GetVariable("_UnregisterInvalidCalled");
                    if (loc_unregistered && (loc_unregistered->GetSInt() == 2))
                    {
                        DEBUG("ValidateInvalidDevices - Device 0x{:016X} is removed. Removing from cache",loc_handle)
                        _cache.erase(loc_handle);
                        _removeddevices.push_back({loc_object});
                        return;
                    }


                    loc_var1->SetUInt(loc_handle & 0xFFFFFFFF);
                    loc_var2->SetUInt((loc_handle >> 32) & 0xFFFFFFFF);
        
                    _cache[loc_handle].object   = loc_object;
                    _cache[loc_handle].id       = loc_id;
                    _cache[loc_handle].rd       = GetScriptVariable<RE::TESObjectARMO>(loc_object,"_DeviceRendered",RE::FormType::Armor);
                    if (!_cache[loc_handle].rd && loc_id)
                    {
                        auto loc_rd = DeviousDevicesAPI::g_API ? DeviousDevicesAPI::g_API->GetDeviceRender(loc_id) : nullptr;
                        _cache[loc_handle].rd = loc_rd;
                    
                        LOG("Getting render device from DD database => 0x{:08X}",loc_rd ? loc_rd->GetFormID() : 0)
                    
                        auto loc_objvar = loc_object->GetVariable("_DeviceRendered");
                        RE::BSScript::PackHandle(loc_objvar,loc_rd,(RE::VMTypeID)RE::FormType::Armor);
                    }
                    
                    auto loc_wearer = GetScriptVariable<RE::Actor>(loc_object,"Wearer",RE::FormType::ActorCharacter);
                    _cache[loc_handle].wearer   = loc_wearer ? loc_wearer->GetHandle().native_handle() : 0x0U;
        
                    DEBUG("Device awaiting cleanup found {} - 0x{:016X} = 0x{:08X} + 0x{:08X} - Wearer = {}",loc_id->GetName(),loc_handle,loc_var1->GetUInt(),loc_var2->GetUInt(),loc_wearer ? loc_wearer->GetName() : "NONE")

                    //ready function args
                    auto loc_args = RE::MakeFunctionArguments();
                    
                    //init unused callback
                    RE::BSTSmartPointer<RE::BSScript::IStackCallbackFunctor> loc_callback;
                    
                    //call papyrus method
                    loc_vm->DispatchMethodCall(loc_object,"_UnregisterInvalid",loc_args,loc_callback);
                    delete loc_args;

                    //LOG("Handle of {} set to 0x{:016X} = 0x{:08X} + 0x{:08X} - Wearer = {}",loc_id->GetName(),a_script->handle,loc_var1->GetUInt(),loc_var2->GetUInt(),loc_wearer ? loc_wearer->GetName() : "NONE")
                    return;
                }
            }
        }
    });
}

RE::BSScript::ObjectTypeInfo* UD::PapyrusDelegate::HaveScriptBase(RE::BSTSmallSharedArray<RE::BSScript::Internal::AttachedScript>& a_scripts, const std::string& a_base) const
{
    for (auto&& it : a_scripts)
    {
        auto loc_info = it->GetTypeInfo();

        while (loc_info != nullptr)
        {
            std::string loc_scriptname = loc_info->GetName();
            std::transform(loc_scriptname.begin(), loc_scriptname.end(), loc_scriptname.begin(), ::tolower);
                
            //check if script is ud
            if (loc_scriptname == a_base)
            {
                return loc_info;
            }
            else
            {
                loc_info = loc_info->GetParent();
            }
        }
    }
    return nullptr;
}

RE::BSScript::ObjectTypeInfo* UD::PapyrusDelegate::HaveScriptBase(RE::BSScript::ObjectTypeInfo* a_type, const std::string& a_base) const
{
    auto loc_info = a_type;

    while (loc_info != nullptr)
    {
        std::string loc_scriptname = loc_info->GetName();
        std::transform(loc_scriptname.begin(), loc_scriptname.end(), loc_scriptname.begin(), ::tolower);
                
        //check if script is ud
        if (loc_scriptname == a_base)
        {
            return loc_info;
        }
        else
        {
            loc_info = loc_info->GetParent();
        }
    }
    return nullptr;
}

FilterDeviceResult PapyrusDelegate::CheckRegisterDevice(RE::VMHandle a_handle,RE::BSScript::ObjectTypeInfo* a_type,RE::Actor* a_actor, std::vector<RE::TESObjectARMO*>& a_devices)
{
    static const auto loc_vm = InternalVM::GetSingleton();

    //get script object
    RE::BSTSmartPointer<RE::BSScript::Object> loc_object = nullptr;
    loc_vm->FindBoundObject(a_handle,a_type->GetName(),loc_object);

    if (loc_object != nullptr)
    {
        //undef this stupidass macro so we can use the GetObject method
        #undef GetObject

        //get wearer from script
        const RE::Actor* loc_wearer = GetScriptVariable<RE::Actor>(loc_object,"Wearer",RE::FormType::ActorCharacter);

        //check if device wearer is the same one as passed actor
        if (loc_wearer == a_actor)
        {
            //get render device from script
            RE::TESObjectARMO* loc_rd = GetScriptVariable<RE::TESObjectARMO>(loc_object,"_DeviceRendered",RE::FormType::Armor);

            if (loc_rd != nullptr)
            {
                //check if current device is one in passed array
                if (std::find(a_devices.begin(),a_devices.end(),loc_rd) != a_devices.end())
                {
                    //get inventory device from papyrus script
                    RE::TESObjectARMO* loc_id = GetScriptProperty<RE::TESObjectARMO>(loc_object,"DeviceInventory",RE::FormType::Armor);
                    
                    LOG("Device {} found",loc_id ? ((RE::TESObjectARMO*)loc_id)->GetName() : "NONE")
                    
                    //ready function args
                    auto loc_args = new RE::BSScript::FunctionArguments<void, RE::Actor*, RE::TESObjectARMO*, RE::TESObjectARMO*>(std::forward<RE::Actor*>(a_actor),std::forward<RE::TESObjectARMO*>(loc_id),std::forward<RE::TESObjectARMO*>(loc_rd));
                    
                    //init unused callback
                    RE::BSTSmartPointer<RE::BSScript::IStackCallbackFunctor> loc_callback;
                    
                    //call papyrus method
                    loc_vm->DispatchMethodCall(loc_object,"RegisterDevice",loc_args,loc_callback);
                    delete loc_args;

                    return {true,loc_id,loc_rd};
                }
            }
        }
    }
    //not found, return default struct
    return {false,nullptr,nullptr};
}

FilterDeviceResult PapyrusDelegate::ProcessDevice(RE::VMHandle a_handle,RE::VMHandle a_handle2, RE::BSScript::ObjectTypeInfo* a_type, RE::Actor* a_actor, std::vector<RE::TESObjectARMO*>& a_devices, std::function<void(RE::BSTSmartPointer<RE::BSScript::Object>,RE::TESObjectARMO*,RE::TESObjectARMO*)> a_fun)
{
    static const auto loc_vm = InternalVM::GetSingleton();

    //get script object
    RE::BSTSmartPointer<RE::BSScript::Object> loc_object = nullptr;
    loc_vm->FindBoundObject(a_handle,a_type->GetName(),loc_object);

    if (loc_object != nullptr && (a_handle2 == 0 || a_handle == a_handle2))
    {
        //undef this stupidass macro so we can use the GetObject method
        #undef GetObject

        //get wearer from script
        const RE::Actor* loc_wearer = GetScriptVariable<RE::Actor>(loc_object,"Wearer",RE::FormType::ActorCharacter);
    
        //check if device wearer is the same one as passed actor
        if (loc_wearer == a_actor)
        {
            //get render device from script
            RE::TESObjectARMO* loc_rd = GetScriptVariable<RE::TESObjectARMO>(loc_object,"_DeviceRendered",RE::FormType::Armor);
            if (loc_rd != nullptr)
            {
                //check if current device is one in passed array
                if (std::find(a_devices.begin(),a_devices.end(),loc_rd) != a_devices.end())
                {
                    //get inventory device from papyrus script
                    auto loc_id = GetScriptProperty<RE::TESObjectARMO>(loc_object,"DeviceInventory",RE::FormType::Armor);
                    
                    if (loc_id != nullptr)
                    {
                        LOG("Device {} found",loc_id ? ((RE::TESObjectARMO*)loc_id)->GetName() : "NONE")
                    
                        //call function
                        a_fun(loc_object,loc_id,loc_rd);

                        return {true,loc_id,loc_rd};
                    }
                }
            }
        }
    }
    //not found, return default struct
    return {false,nullptr,nullptr};
}

FilterDeviceResult UD::PapyrusDelegate::ProcessDevice2(RE::VMHandle a_handle, RE::VMHandle a_handle2, RE::BSScript::ObjectTypeInfo* a_type, RE::TESObjectARMO* a_device, std::function<bool(RE::BSTSmartPointer<RE::BSScript::Object>, RE::TESObjectARMO*, RE::TESObjectARMO*)> a_fun)
{
    static const auto loc_vm = InternalVM::GetSingleton();

    //get script object
    RE::BSTSmartPointer<RE::BSScript::Object> loc_object = nullptr;
    loc_vm->FindBoundObject(a_handle,a_type->GetName(),loc_object);

    if (loc_object != nullptr && (a_handle2 == 0 || a_handle == a_handle2))
    {
        //undef this stupidass macro so we can use the GetObject method
        #undef GetObject

        //get render device from script
        RE::TESObjectARMO* loc_rd = GetScriptVariable<RE::TESObjectARMO>(loc_object,"_DeviceRendered",RE::FormType::Armor);
        if (loc_rd != nullptr)
        {
            //check if current device is one in passed array
            if (a_device == loc_rd)
            {
                //get inventory device from papyrus script
                auto loc_id = GetScriptProperty<RE::TESObjectARMO>(loc_object,"DeviceInventory",RE::FormType::Armor);
                    
                if (loc_id != nullptr)
                {
                    //LOG("Device {} found",loc_id ? ((RE::TESObjectARMO*)loc_id)->GetName() : "NONE")
                    
                    //call function
                    const bool loc_res = a_fun(loc_object,loc_id,loc_rd);
                    return {loc_res,loc_id,loc_rd};
                }
            }
        }
        else
        {
            LOG("Object doesnt have set RD")
        }
    }
    else
    {
        LOG("Cant find object with VMHandle {}",a_handle)
    }
    //not found, return default struct
    return {false,nullptr,nullptr};
}

bool UD::PapyrusDelegate::GetDeviceScript(RE::Actor* a_actor, RE::TESObjectARMO* a_device, std::string a_script, std::string a_variable)
{
    LOG("GetDeviceScript({},0x{:08X},{},{}) - Called",a_actor ? a_actor->GetName() : "NONE",a_device ? a_device->GetFormID() : 0,a_script,a_variable)
    
    if (!a_actor || !a_device || a_script == "" || a_variable == "")
    {
        return false;
    }

    // Find device script
    Object loc_device = FindDeviceScript(a_actor,a_device);

    if (loc_device == nullptr)
    {
        ERROR("GetDeviceScript({},0x{:08X},{},{}) - Could not find script object",a_actor->GetName(),a_device->GetFormID(),a_script,a_variable)
        return false;
    }

    std::transform(a_script.begin(), a_script.end(), a_script.begin(), ::tolower);

    for (auto&& [script,scriptVM] : _modscripts)
    {
        std::string loc_script = script.scriptname;
        std::transform(loc_script.begin(), loc_script.end(), loc_script.begin(), ::tolower);
        if (loc_script == a_script)
        {
            auto loc_object = scriptVM.object;
            if (loc_object != nullptr)
            {
                Variable* loc_var = loc_object->GetProperty(a_variable) ? loc_object->GetProperty(a_variable) : loc_object->GetVariable(a_variable);
                if (loc_var != nullptr)
                {
                    loc_var->SetObject(loc_device,loc_var->GetType().GetRawType());
                    return true;
                }
                else
                {
                    ERROR("GetDeviceScript({},0x{:08X},{},{}) - Could not find quest variable",a_actor->GetName(),a_device->GetFormID(),a_script,a_variable)
                }

                break;
            }
            else
            {
                ERROR("GetDeviceScript({},0x{:08X},{},{}) - Could not find quest script",a_actor->GetName(),a_device->GetFormID(),a_script,a_variable)
            }
        }
    }

    return false;
}

bool UD::PapyrusDelegate::GetInventoryDeviceScript(RE::Actor* a_actor, RE::TESObjectARMO* a_device, std::string a_script, std::string a_variable)
{
    LOG("GetDeviceScript({},0x{:08X},{},{}) - Called",a_actor ? a_actor->GetName() : "NONE",a_device ? a_device->GetFormID() : 0,a_script,a_variable)
    
    if (!a_actor || !a_device || a_script == "" || a_variable == "")
    {
        return false;
    }

    // Find device script
    Object loc_device = FindInventoryDeviceScript(a_actor,a_device);

    if (loc_device == nullptr)
    {
        ERROR("GetInventoryDeviceScript({},0x{:08X},{},{}) - Could not find script object",a_actor->GetName(),a_device->GetFormID(),a_script,a_variable)
        return false;
    }

    std::transform(a_script.begin(), a_script.end(), a_script.begin(), ::tolower);

    for (auto&& [script,scriptVM] : _modscripts)
    {
        std::string loc_script = script.scriptname;
        std::transform(loc_script.begin(), loc_script.end(), loc_script.begin(), ::tolower);
        if (loc_script == a_script)
        {
            auto loc_object = scriptVM.object;
            if (loc_object != nullptr)
            {
                Variable* loc_var = loc_object->GetProperty(a_variable) ? loc_object->GetProperty(a_variable) : loc_object->GetVariable(a_variable);
                if (loc_var != nullptr)
                {
                    loc_var->SetObject(loc_device,loc_var->GetType().GetRawType());
                    return true;
                }
                else
                {
                    ERROR("GetInventoryDeviceScript({},0x{:08X},{},{}) - Could not find quest variable",a_actor->GetName(),a_device->GetFormID(),a_script,a_variable)
                }

                break;
            }
            else
            {
                ERROR("GetInventoryDeviceScript({},0x{:08X},{},{}) - Could not find quest script",a_actor->GetName(),a_device->GetFormID(),a_script,a_variable)
            }
        }
    }

    return false;
}

void UD::PapyrusDelegate::ResetCache()
{
    Utils::UniqueLock lock(_SaveLock);
    _cache.clear();
    _removeddevices.clear();
}

UD::Object UD::PapyrusDelegate::FindDeviceScript(RE::Actor* a_actor, RE::TESObjectARMO* a_device)
{
    if (!a_actor || !a_device) return Object();

    const auto loc_vm = InternalVM::GetSingleton();
    auto loc_formid = a_actor->GetFormID();
     
    loc_vm->attachedScriptsLock.Lock();

    RE::VMHandle loc_handle = 0x0001000000000000 | loc_formid;

    const RE::VMHandle loc_scriptstep = 0x100000000;
    Object loc_object;

    int loc_nonefound = 15;
    while (loc_nonefound > 0)
    {
        loc_vm->FindBoundObject(loc_handle,"ud_customdevice_renderscript",loc_object);
        DEBUG("0x{:016X} = {}",loc_handle,loc_object ? loc_object->GetTypeInfo()->GetName() : "NONE")
        if (loc_object)
        {
            auto loc_rd = GetScriptVariable<RE::TESObjectARMO>(loc_object,"_DeviceRendered",RE::FormType::Armor);
            if (loc_rd == a_device)
            {
                break;
            }
            loc_nonefound = 15;
        }
        else
        {
            loc_nonefound--;
        }
        loc_handle += loc_scriptstep;
    }
    
    loc_vm->attachedScriptsLock.Unlock();
    return loc_object;
}

UD::Object UD::PapyrusDelegate::FindInventoryDeviceScript(RE::Actor* a_actor, RE::TESObjectARMO* a_device)
{
    if (!a_actor || !a_device) return Object();

    const auto loc_vm = InternalVM::GetSingleton();
    auto loc_formid = a_actor->GetFormID();
     
    loc_vm->attachedScriptsLock.Lock();

    RE::VMHandle loc_handle = 0x0001000000000000 | loc_formid;

    const RE::VMHandle loc_scriptstep = 0x100000000;
    Object loc_object;

    int loc_nonefound = 15;
    while (loc_nonefound > 0)
    {
        loc_vm->FindBoundObject(loc_handle,"ud_customdevice_equipscript",loc_object);
        if (loc_object)
        {
            auto loc_id = GetScriptProperty<RE::TESObjectARMO>(loc_object,"deviceInventory",RE::FormType::Armor);
            if (loc_id == a_device)
            {
                break;
            }
            loc_nonefound = 15;
        }
        else
        {
            loc_nonefound--;
        }
        loc_handle += loc_scriptstep;
    }
    
    loc_vm->attachedScriptsLock.Unlock();
    return loc_object;
}

std::vector<UD::Object> UD::PapyrusDelegate::FindAllDeviceScripts(RE::Actor* a_actor)
{
    if (!a_actor) return std::vector<Object>();

    const auto loc_vm = InternalVM::GetSingleton();
    auto loc_formid = a_actor->GetFormID();
     
    loc_vm->attachedScriptsLock.Lock();

    RE::VMHandle loc_handle = 0x0001000000000000 | loc_formid;

    const RE::VMHandle loc_scriptstep = 0x100000000;
    std::vector<Object> loc_objects;

    int loc_nonefound = 15;
    while (loc_nonefound > 0)
    {
        Object loc_object;
        loc_vm->FindBoundObject(loc_handle,"form",loc_object);
        if (loc_object)
        {
            auto loc_info = loc_object->GetTypeInfo();
            bool loc_isud = false;
            while (loc_info != nullptr)
            {
                std::string loc_scriptname = loc_info->GetName();
                std::transform(loc_scriptname.begin(), loc_scriptname.end(), loc_scriptname.begin(), ::tolower);
                
                //check if script is ud
                if (loc_scriptname == "ud_customdevice_renderscript")
                {
                    loc_isud = true;
                    break;
                }
                else
                {
                    loc_info = loc_info->GetParent();
                }
            }

            if (loc_isud)
            {
                loc_objects.push_back(loc_object);
            }
            loc_nonefound = 15;
        }
        else
        {
            loc_nonefound--;
        }
        loc_handle += loc_scriptstep;
    }
    
    loc_vm->attachedScriptsLock.Unlock();
    return loc_objects;
}

void UD::PapyrusDelegate::UpdateVMHandles() const
{
    DEBUG("UpdateVMHandles called")

    const auto loc_vm = InternalVM::GetSingleton();

    loc_vm->attachedScriptsLock.Lock();
    std::for_each(std::execution::seq,loc_vm->attachedScripts.begin(),loc_vm->attachedScripts.end(),[&](Script& a_script)
    {
        auto loc_type = HaveScriptBase(a_script.second,"ud_customdevice_renderscript");
        if (loc_type != nullptr)
        {
            RE::VMHandle loc_handle = a_script.first;
            //get script object
            RE::BSTSmartPointer<RE::BSScript::Object> loc_object = nullptr;

            loc_vm->FindBoundObject(loc_handle,loc_type->GetName(),loc_object);

            if (loc_object != nullptr)
            {
                //undef this stupidass macro so we can use the GetObject method
                #undef GetObject

                //get inventory device from papyrus script
                auto loc_id = GetScriptProperty<RE::TESObjectARMO>(loc_object,"DeviceInventory",RE::FormType::Armor);
                if (loc_id != nullptr)
                {
                    const auto loc_var1 = loc_object->GetVariable("_VMHandle1");
                    const auto loc_var2 = loc_object->GetVariable("_VMHandle2");

                    if (loc_var1 == nullptr || loc_var2 == nullptr) return;

                    loc_var1->SetUInt(loc_handle & 0xFFFFFFFF);
                    loc_var2->SetUInt((loc_handle >> 32) & 0xFFFFFFFF);

                    _cache[loc_handle].object   = loc_object;
                    _cache[loc_handle].id       = loc_id;
                    _cache[loc_handle].rd       = GetScriptVariable<RE::TESObjectARMO>(loc_object,"_DeviceRendered",RE::FormType::Armor);
                    if (!_cache[loc_handle].rd && loc_id)
                    {
                        auto loc_rd = DeviousDevicesAPI::g_API->GetDeviceRender(loc_id);
                        _cache[loc_handle].rd = loc_rd;

                        LOG("Getting render device from DD database => 0x{:08X}",loc_rd ? loc_rd->GetFormID() : 0)

                        auto loc_objvar = loc_object->GetVariable("_DeviceRendered");
                        RE::BSScript::PackHandle(loc_objvar,loc_rd,(RE::VMTypeID)RE::FormType::Armor);
                    }

                    auto loc_wearer = GetScriptVariable<RE::Actor>(loc_object,"Wearer",RE::FormType::ActorCharacter);
                    _cache[loc_handle].wearer   = loc_wearer ? loc_wearer->GetHandle().native_handle() : 0x0U;

                    LOG("Handle of {} set to 0x{:016X} = 0x{:08X} + 0x{:08X} - Wearer = {}",loc_id->GetName(),a_script.first,loc_var1->GetUInt(),loc_var2->GetUInt(),loc_wearer ? loc_wearer->GetName() : "NONE")
                    return;
                }
            }
        }

        auto loc_modtype = HaveScriptBase(a_script.second,"ud_modifier");
        if (loc_modtype != nullptr)
        {
            RE::VMHandle loc_handle = a_script.first;
            //get script object
            RE::BSTSmartPointer<RE::BSScript::Object> loc_object = nullptr;

            loc_vm->FindBoundObject(loc_handle,loc_modtype->GetName(),loc_object);

            if (loc_object != nullptr)
            {
                //undef this stupidass macro so we can use the GetObject method
                #undef GetObject

                _modifiercache[loc_handle].object   = loc_object;
                _modifiercache[loc_handle].name     = loc_object->GetProperty("NameFull")->GetString();
                _modifiercache[loc_handle].namealias    = loc_object->GetProperty("NameAlias")->GetString();

                const RE::FormID loc_formid = static_cast<RE::FormID>(loc_handle & 0x00000000FFFFFFFF);

                auto loc_quest = reinterpret_cast<RE::TESQuest*>(RE::TESForm::LookupByID(loc_formid));
                _modifiercache[loc_handle].quest = loc_quest;

                const uint32_t loc_qalias = static_cast<uint32_t>((loc_handle >> 32U));
                _modifiercache[loc_handle].alias = loc_quest->aliases[loc_qalias];;

                //DEBUG("Modifier {}::{} found",_modifiercache[loc_handle].alias,_modifiercache[loc_handle].name)
                return;
            }
        }

        auto loc_npcslottype = HaveScriptBase(a_script.second,"ud_customdevice_npcslot");
        if (loc_npcslottype != nullptr)
        {
            RE::VMHandle loc_handle = a_script.first;
            //get script object
            RE::BSTSmartPointer<RE::BSScript::Object> loc_object = nullptr;

            loc_vm->FindBoundObject(loc_handle,loc_npcslottype->GetName(),loc_object);

            if (loc_object != nullptr)
            {
                //undef this stupidass macro so we can use the GetObject method
                #undef GetObject

                _npcslotcache[loc_handle].object   = loc_object;

                const RE::FormID loc_formid = static_cast<RE::FormID>(loc_handle & 0x00000000FFFFFFFF);

                auto loc_quest = reinterpret_cast<RE::TESQuest*>(RE::TESForm::LookupByID(loc_formid));
                _npcslotcache[loc_handle].quest = loc_quest;

                if (loc_quest == nullptr)
                {
                    DEBUG("Quest is none ??? Skipping")
                    _npcslotcache.erase(loc_handle);
                    return;
                }

                const uint32_t loc_qalias = static_cast<uint32_t>((loc_handle >> 32U));

                if (loc_qalias >= loc_quest->aliases.size())
                {
                    DEBUG("Alias is out of the range ??? Skipping")
                    _npcslotcache.erase(loc_handle);
                    return;
                }

                if (loc_quest->aliases[loc_qalias] == nullptr)
                {
                    DEBUG("Alias is none ??? Skipping")
                    _npcslotcache.erase(loc_handle);
                    return;
                }

                if (loc_quest->aliases[loc_qalias]->GetVMTypeID() != 140)
                {
                    DEBUG("Alias is not ReferenceAlias ?? Skipping")
                    _npcslotcache.erase(loc_handle);
                    return;
                }

                _npcslotcache[loc_handle].alias = reinterpret_cast<RE::BGSRefAlias*>(loc_quest->aliases[loc_qalias]);
                _npcslotcache[loc_handle].id = loc_qalias;

                //{
                //    //auto loc_regdevices = loc_object->GetVariable("_iUsedSlots")->GetSInt();
                //    //DEBUG("0x{:016X}, loc_regdevices = {}",loc_handle,loc_regdevices)
                //    auto loc_arr = loc_object->GetProperty("UD_equipedCustomDevices")->GetArray();
                //    if (loc_arr)
                //    {
                //        for(size_t i = 0; i < loc_arr->size();i++)
                //        {
                //            auto loc_vartype = (*loc_arr)[i].GetType();
                //            //DEBUG("Type = {}, RawType = {}",(size_t)loc_vartype.GetUnmangledRawType(),(size_t)loc_vartype.GetRawType())
                //
                //            auto loc_device = (*loc_arr)[i].GetObject();
                //            DEBUG("0x{:016X} - {}",loc_device ? loc_device->handle : 0x0,loc_device ? loc_device->GetTypeInfo()->GetName() : "NONE")
                //        }
                //    }
                //}

                //DEBUG("Modifier {}::{} found",_modifiercache[loc_handle].alias,_modifiercache[loc_handle].name)
                return;
            }
        }

    });
    loc_vm->attachedScriptsLock.Unlock();

    ValidateCache();

    DEBUG("Cache size: {}",_cache.size())
    for(auto&& [vmhandle,device] : _cache) 
    {
        RE::Actor* loc_wearer = RE::Actor::LookupByHandle(device.wearer).get();
        DEBUG("0x{:016X} - {}(0x{:08X}) / 0x{:08X} = {} ; Wearer = {}",vmhandle,device.id->GetName(),device.id->GetFormID(),device.rd ? device.rd->GetFormID() : 0x0,device.object->type->name,loc_wearer ? loc_wearer->GetName() : "NONE")
    }

    DEBUG("Modifier cache size: {}",_modifiercache.size())
    for(auto&& [vmhandle,modifier] : _modifiercache) 
    {
        DEBUG("0x{:016X} - {}::{} , Storage = {} , Alias = {}",vmhandle,modifier.namealias,modifier.name,modifier.quest ? modifier.quest->GetName() : "NONE", modifier.alias ? modifier.alias->aliasName : "NONE")
    }

    DEBUG("NPC Slot cache size: {}",_npcslotcache.size())
    for(auto&& [vmhandle,npcslot] : _npcslotcache) 
    {
        DEBUG("0x{:08X} - 0x{:016X}, Actor = {}, Storage = {} , Alias = {}",npcslot.id,vmhandle,npcslot.alias ? (npcslot.alias->GetActorReference() ? npcslot.alias->GetActorReference()->GetName() : "NONE") : "NONE",npcslot.quest ? npcslot.quest->GetName() : "NONE", npcslot.alias ? npcslot.alias->aliasName : "NONE")
    }
}

UD::Device UD::PapyrusDelegate::GetDeviceScript(int a_handle1, int a_handle2, RE::TESObjectARMO* a_device)
{
    RE::VMHandle loc_vmhandle = ToVMHandle(a_handle1,a_handle2);
    loc_vmhandle = ValidateVMHandle(loc_vmhandle,a_device);

    ValidateCache();
    auto loc_cacheres = _cache[loc_vmhandle];

    return loc_cacheres;
}

UD::Device UD::PapyrusDelegate::GetCachedDevice(RE::VMHandle a_handle, RE::Actor* a_actor, RE::TESObjectARMO* a_device)
{
    RE::VMHandle loc_vmhandle = a_handle;
    loc_vmhandle = ValidateVMHandle(loc_vmhandle,a_device);

    ValidateCache();
    Device loc_cacheres = _cache[loc_vmhandle];
    return loc_cacheres;
}

const std::map<RE::VMHandle, UD::Modifier>& UD::PapyrusDelegate::GetModifiers() const
{
    return _modifiercache;
}

std::vector<UD::Modifier> UD::PapyrusDelegate::GetModifiers(RE::VMHandle a_handle, RE::TESObjectARMO* a_device)
{
    auto loc_modifiers = GetDeviceAliasArray(a_handle,a_device,"UD_ModifiersRef");

    std::string loc_param;

    std::vector<UD::Modifier> loc_res;

    for (int i = 0; i < loc_modifiers.size(); i++)
    {
        if (loc_modifiers[i] != nullptr)
        {
            auto loc_mod = PapyrusDelegate::GetSingleton()->GetModifier(loc_modifiers[i]);
            if (loc_mod.name == "ERROR") continue;
            else
            {
                loc_res.push_back(loc_mod);
            }
        }
        else
        {
            ERROR("Alias is NULL")
        }
    }

    return loc_res;
}

UD::Modifier UD::PapyrusDelegate::GetModifier(RE::BGSBaseAlias* a_alias) const
{
    auto loc_res = std::find_if(_modifiercache.begin(),_modifiercache.end(),[&](std::pair<const RE::VMHandle,Modifier> a_mod)
    {
        if (a_mod.second.alias == a_alias) return true;
        else return false;
    });
    return loc_res != _modifiercache.end() ? loc_res->second : UD::Modifier();
}

UD::Modifier UD::PapyrusDelegate::GetModifier(std::string a_namealias) const
{
    auto loc_res = std::find_if(_modifiercache.begin(),_modifiercache.end(),[&](std::pair<const RE::VMHandle,Modifier> a_mod)
    {
        if (a_mod.second.namealias == a_namealias || a_mod.second.name == a_namealias) return true;
        else return false;
    });
    return loc_res != _modifiercache.end() ? loc_res->second : UD::Modifier();
}

std::vector<RE::BGSBaseAlias*> UD::PapyrusDelegate::GetDeviceAliasArray(RE::VMHandle a_handle, RE::TESObjectARMO* a_device, std::string a_pname)
{
    RE::VMHandle loc_vmhandle = a_handle;
    loc_vmhandle = ValidateVMHandle(loc_vmhandle,a_device);

    ValidateCache();
    Device loc_cacheres = _cache[loc_vmhandle];

    // Alias is not form. Because of that, the type is 139 and 140 (dunno why it have 2 types) (while there are only 138 form types)
    auto loc_res = GetScriptFormArray<RE::BGSBaseAlias>(loc_cacheres.object,a_pname,(RE::FormType)139);

    return loc_res;
}

std::vector<std::string> UD::PapyrusDelegate::GetDeviceStringArray(RE::VMHandle a_handle, RE::TESObjectARMO* a_device, std::string a_pname)
{
    RE::VMHandle loc_vmhandle = a_handle;
    loc_vmhandle = ValidateVMHandle(loc_vmhandle,a_device);

    ValidateCache();
    Device loc_cacheres = _cache[loc_vmhandle];

    if (loc_cacheres.object == nullptr) return std::vector<std::string>();

    const auto loc_var = loc_cacheres.object->GetProperty(a_pname);

    auto loc_vararrptr = loc_var->GetArray();

    if (loc_vararrptr == nullptr) return std::vector<std::string>();

    auto loc_vararr = loc_vararrptr.get();

    std::vector<std::string> loc_res;
    for (uint8_t i = 0; i < loc_vararr->size(); i++)
    {
        //undef this stupidass macro so we can use the GetObject method
        #undef GetObject
        auto loc_varobj = (std::string)(*loc_vararr)[i].GetString();
        loc_res.push_back(loc_varobj);
    }
    return loc_res;
}

RE::BSTSmartPointer<RE::BSScript::Object> UD::PapyrusDelegate::GetScript(std::string a_script)
{
    for (auto&& [form,formVM] : _modscripts)
    {
        if (form.scriptname == a_script) return formVM.object;
    }
    return RE::BSTSmartPointer<RE::BSScript::Object>();
}

bool UD::PapyrusDelegate::GetScriptVariableBool(std::string a_script,std::string a_variable,bool a_property)
{
    bool loc_res = false;
    auto loc_script = GetScript(a_script);
    if (loc_script)
    {
        auto loc_var = a_property ? loc_script->GetProperty(a_variable) : loc_script->GetVariable(a_variable);
        loc_res = loc_var ? loc_var->GetBool() : false;
    }
    return loc_res;
}

void UD::PapyrusDelegate::Lock()
{
    _SaveLock.Lock();
}

void UD::PapyrusDelegate::Unlock()
{
    _SaveLock.Unlock();
}

template<class T>
T* PapyrusDelegate::GetScriptVariable(RE::BSTSmartPointer<RE::BSScript::Object> a_scriptobject, RE::BSFixedString a_variable, RE::FormType a_type) const
{
    if (a_scriptobject == nullptr) return nullptr;

    const auto loc_var = a_scriptobject->GetVariable(a_variable);

    if (loc_var == nullptr) return nullptr;

    const auto loc_varobj = loc_var->GetObject();

    if (loc_varobj == nullptr) return nullptr;

    return static_cast<T*>(loc_varobj->Resolve((RE::VMTypeID)a_type));
}

template<class T>
T* PapyrusDelegate::GetScriptProperty(RE::BSTSmartPointer<RE::BSScript::Object> a_scriptobject, RE::BSFixedString a_property, RE::FormType a_type) const
{
    if (a_scriptobject == nullptr) return nullptr;

    const auto loc_var = a_scriptobject->GetProperty(a_property);

    if (loc_var == nullptr) return nullptr;

    const auto loc_varobj = loc_var->GetObject();

    if (loc_varobj == nullptr) return nullptr;

    return static_cast<T*>(loc_varobj->Resolve((RE::VMTypeID)a_type));
}