#include <UD_PapyrusDelegate.h>

#undef GetObject

SINGLETONBODY(UD::PapyrusDelegate)

using UD::PapyrusDelegate;
typedef PapyrusDelegate::Result Result;
typedef PapyrusDelegate::FilterDeviceResult FilterDeviceResult;

bool UD::ThreadLock::_used = false;

RE::VMHandle UD::PapyrusDelegate::ToVMHandle(const int a_1, const int a_2)
{
    const uint64_t loc_fir =  (*(uint32_t*)&a_1);
    const uint64_t loc_sec = ((uint64_t)(*(uint32_t*)&a_2) << 32);
    return loc_fir | loc_sec;
}

void PapyrusDelegate::Setup()
{
    if (!_installed)
    {
        _udrdkw = reinterpret_cast<RE::BGSKeyword*>(RE::TESDataHandler::GetSingleton()->LookupForm(0x11A352,"UnforgivingDevices.esp"));

        HINSTANCE dllHandle = LoadLibrary(TEXT("DeviousDevices.dll"));
        if (dllHandle != NULL)
        {
            FARPROC pGetDeviceRender = GetProcAddress(HMODULE (dllHandle),"GetDeviceRender");
            DDNGGetDeviceRender = GetDeviceRender(pGetDeviceRender);
            DEBUG("PapyrusDelegate::Setup() - DDNGGetDeviceRender imported - addrs = 0x{:016X}",(uintptr_t)DDNGGetDeviceRender)
            //FreeLibrary(dllHandle);
        }
        else
        {
            ERROR("PapyrusDelegate::Setup() - ERROR: Cant find DeviousDevices.dll!!")
            DDNGGetDeviceRender = nullptr;
        }
        _installed = true;
        LOG("PapyrusDelegate::Setup - installed")
    }
}

void UD::PapyrusDelegate::Reload()
{
    Setup();
    _cache.clear();
    UpdateVMHandles();
}

int PapyrusDelegate::SendRegisterDeviceScriptEvent(RE::Actor* a_actor, std::vector<RE::TESObjectARMO*>& a_devices)
{
    LOG("SendRegisterDeviceScriptEvent called")
    if (a_actor == nullptr) return -1;

    if (!_udrdkw) 
    {
        ERROR("ERROR: UD RD keyword is none !")
        return 0;
    }

    std::erase_if(a_devices,[this](const RE::TESObjectARMO* a_device)
    {
        if ((a_device != nullptr) && !a_device->HasKeyword(_udrdkw)) return true;
        return false;
    });

    size_t loc_tofound = a_devices.size();
    if (loc_tofound == 0) return 0;

    LOG("Finding scripts for {} devices",loc_tofound)

    const auto loc_vm = InternalVM::GetSingleton();
    
    ValidateCache();

    for(auto rd : a_devices)
    {
        for (auto&& [vmhandle,cached] : _cache)
        {
            if (cached.rd == rd && rd)
            {
                //get wearer from script
                RE::Actor* loc_wearer = RE::Actor::LookupByHandle(cached.wearer).get();

                //check if device wearer is the same one as passed actor
                if (loc_wearer == a_actor)
                {
                    //get inventory device from papyrus script
                    RE::TESObjectARMO* loc_id = cached.id;

                    //check if actor really wears the device
                    //if (!Utility::GetSingleton()->CheckArmorEquipped(loc_wearer,rd)) 
                    //{
                    //    LOG("Device {} found, but actor is not wearing it. Skipping!",loc_id ? ((RE::TESObjectARMO*)loc_id)->GetName() : "NONE")
                    //    continue;
                    //}

                    LOG("Device {} found",loc_id ? ((RE::TESObjectARMO*)loc_id)->GetName() : "NONE")
                    
                    //ready function args
                    auto loc_args = new RE::BSScript::FunctionArguments<void, RE::Actor*, RE::TESObjectARMO*, RE::TESObjectARMO*>(std::forward<RE::Actor*>(a_actor),std::forward<RE::TESObjectARMO*>(loc_id),std::forward<RE::TESObjectARMO*>(rd));
                    
                    //init unused callback
                    RE::BSTSmartPointer<RE::BSScript::IStackCallbackFunctor> loc_callback;
                    
                    //call papyrus method
                    loc_vm->DispatchMethodCall(cached.object,"RegisterDevice",loc_args,loc_callback);
                    delete loc_args;

                    loc_tofound--;
                }
            }
        }
    }

    //some device was not found
    return  static_cast<int>(a_devices.size() - loc_tofound);
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

    auto loc_cacheres = _cache[a_handle];
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

            const bool loc_isremoved = (loc_device->GetVariable("_deviceControlBitMap_1")->GetSInt() & 0x04000000U);

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
        auto loc_type = IsUnforgivingDevice(it.second);
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
            auto loc_rd = DDNGGetDeviceRender(cached.id);
            cached.rd = loc_rd;

            LOG("Getting render device from DD database => 0x{:08X}",loc_rd ? loc_rd->GetFormID() : 0)

            auto loc_objvar = cached.object->GetVariable("_DeviceRendered");
            RE::BSScript::PackHandle(loc_objvar,loc_rd,(RE::VMTypeID)RE::FormType::Armor);
        }

        if (cached.object->refCountAndHandleLock > 2) continue; //if number of references is 2, it means that no thread is currently running on the object (hopefully)

        auto loc_var = cached.object->GetVariable("_deviceControlBitMap_1");
        if (loc_var == nullptr) continue;

        int32_t loc_val = loc_var->GetSInt();
        if (loc_val & 0x04000000)
        {
            loc_toremove.push_back(vmhandle);
            LOG("PapyrusDelegate::ValidateCache - {} is no longer valid - Removing from cache",cached.id->GetName())
        }
    }

    for(auto&& it : loc_toremove) _cache.erase(it);
}

RE::BSScript::ObjectTypeInfo* PapyrusDelegate::IsUnforgivingDevice(RE::BSTSmallSharedArray<RE::BSScript::Internal::AttachedScript>& a_scripts) const
{
    for (auto&& it : a_scripts)
    {
        auto loc_info = it->GetTypeInfo();

        while (loc_info != nullptr)
        {
            std::string loc_scriptname = loc_info->GetName();
            std::transform(loc_scriptname.begin(), loc_scriptname.end(), loc_scriptname.begin(), ::tolower);
                
            //check if script is ud
            if (loc_scriptname == UDBASESCRIPT)
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

void UD::PapyrusDelegate::UpdateVMHandles() const
{
    LOG("UpdateVMHandles called")

    const auto loc_vm = InternalVM::GetSingleton();

    Spinlock loc_lock;

    loc_vm->attachedScriptsLock.Lock();
    std::for_each(std::execution::seq,loc_vm->attachedScripts.begin(),loc_vm->attachedScripts.end(),[&](Script& a_script)
    {
        auto loc_type = IsUnforgivingDevice(a_script.second);
        if (loc_type != nullptr)
        {
            RE::VMHandle loc_handle = a_script.first;
            //get script object
            RE::BSTSmartPointer<RE::BSScript::Object> loc_object = nullptr;

            UniqueLock lock(loc_lock);
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
                        auto loc_rd = DDNGGetDeviceRender(loc_id);
                        _cache[loc_handle].rd = loc_rd;

                        LOG("Getting render device from DD database => 0x{:08X}",loc_rd ? loc_rd->GetFormID() : 0)

                        auto loc_objvar = loc_object->GetVariable("_DeviceRendered");
                        RE::BSScript::PackHandle(loc_objvar,loc_rd,(RE::VMTypeID)RE::FormType::Armor);
                    }

                    auto loc_wearer = GetScriptVariable<RE::Actor>(loc_object,"Wearer",RE::FormType::ActorCharacter);
                    _cache[loc_handle].wearer   = loc_wearer ? loc_wearer->GetHandle().native_handle() : 0x0U;

                    LOG("Handle of {} set to 0x{:016X} = 0x{:08X} + 0x{:08X} - Wearer = {}",loc_id->GetName(),a_script.first,loc_var1->GetUInt(),loc_var2->GetUInt(),loc_wearer ? loc_wearer->GetName() : "NONE")
                }
            }
        }
    });
    loc_vm->attachedScriptsLock.Unlock();

    DEBUG("Cache size: {}",_cache.size())
    for(auto&& [vmhandle,device] : _cache) 
    {
        RE::Actor* loc_wearer = RE::Actor::LookupByHandle(device.wearer).get();
        DEBUG("0x{:016X} - {}(0x{:08X}) / 0x{:08X} = {} ; Wearer = {}",vmhandle,device.id->GetName(),device.id->GetFormID(),device.rd ? device.rd->GetFormID() : 0x0,device.object->type->name,loc_wearer ? loc_wearer->GetName() : "NONE")
    }
    ValidateCache();
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
