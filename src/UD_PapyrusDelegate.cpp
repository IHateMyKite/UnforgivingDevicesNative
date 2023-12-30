#include <UD_PapyrusDelegate.h>

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
    if (!_installed)
    {
        _udrdkw = reinterpret_cast<RE::BGSKeyword*>(RE::TESDataHandler::GetSingleton()->LookupForm(0x11A352,"UnforgivingDevices.esp"));
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

    for (auto&& [vmhandle,cached] : _cache)
    {
        for(auto rd : a_devices)
        {
            if (cached.rd == rd && rd)
            {
                //get wearer from script
                const RE::Actor* loc_wearer = cached.wearer;

                //check if device wearer is the same one as passed actor
                if (loc_wearer == a_actor)
                {
                    //get inventory device from papyrus script
                    RE::TESObjectARMO* loc_id = cached.id;
                    
                    LOG("Device {} found",loc_id ? ((RE::TESObjectARMO*)loc_id)->GetName() : "NONE")
                    
                    //ready function args
                    auto loc_args = new RE::BSScript::FunctionArguments<void, RE::Actor*, RE::TESObjectARMO*, RE::TESObjectARMO*>(std::forward<RE::Actor*>(a_actor),std::forward<RE::TESObjectARMO*>(loc_id),std::forward<RE::TESObjectARMO*>(rd));
                    
                    //init unused callback
                    RE::BSTSmartPointer<RE::BSScript::IStackCallbackFunctor> loc_callback;
                    
                    //call papyrus method
                    loc_vm->DispatchMethodCall(cached.object,"RegisterDevice",loc_args,loc_callback);

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
        //call papyrus method
        if (a_threads & tStarter)   loc_vm->DispatchMethodCall(loc_cacheres.object,"_MinigameStarterThread",RE::MakeFunctionArguments(),loc_callback1);
        if (a_threads & tCrit)      loc_vm->DispatchMethodCall(loc_cacheres.object,"_MinigameCritLoopThread",RE::MakeFunctionArguments(),loc_callback2);
        if (a_threads & tParalel)   loc_vm->DispatchMethodCall(loc_cacheres.object,"_MinigameParalelThread",RE::MakeFunctionArguments(),loc_callback3);
        if (a_threads & tAV)        loc_vm->DispatchMethodCall(loc_cacheres.object,"_MinigameAVCheckLoopThread",RE::MakeFunctionArguments(),loc_callback4);
        LOG("PapyrusDelegate::SendMinigameThreadEvents - events sent")

        return Result::rSuccess;
    }
    return Result::rNotFound;
}

Result PapyrusDelegate::SendRemoveRenderDeviceEvent(RE::Actor* a_actor, RE::TESObjectARMO* a_device)
{
    LOG("SendRemoveRenderDeviceEvent({},{:08X}) called",a_actor ? a_actor->GetName() : "NONE",a_device ? a_device->GetFormID() : 0x0)
    if (a_actor == nullptr || a_device == nullptr) return Result::rArgError;
    if (!a_device->HasKeyword(_udrdkw)) return Result::rDeviceError;

    std::vector<RE::TESObjectARMO*> loc_device = {a_device};

    const auto loc_vm = InternalVM::GetSingleton();

    ValidateCache();

    for(auto&& [vmhandle,cached] : _cache)
    {
        if (cached.rd == a_device)
        {
            LOG("Device object found in cache - using it")

            const RE::Actor* loc_wearer = GetScriptVariable<RE::Actor>(cached.object,"Wearer",RE::FormType::ActorCharacter);

            if (loc_wearer == a_actor)
            {
                //init unused callback
                RE::BSTSmartPointer<RE::BSScript::IStackCallbackFunctor> loc_callback;
                auto loc_args = new RE::BSScript::FunctionArguments<void, RE::Actor*>(std::forward<RE::Actor*>(a_actor));
                //call papyrus method
                loc_vm->DispatchMethodCall(cached.object,"removeDevice",loc_args,loc_callback);

                LOG("SendRemoveRenderDeviceEvent({},{:08X}) - event sent",a_actor->GetName(),a_device->GetFormID())

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
        LOG("Device object found in cache - using it - Wearer = {}",loc_cacheres.wearer->GetName())

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
    LOG("PapyrusDelegate::ValidateVMHandle - called")

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
                _cache[it.first].wearer  = GetScriptVariable<RE::Actor>(a_object,"Wearer",RE::FormType::ActorCharacter);

                if (loc_var1->GetUInt() == 0 && loc_var2->GetUInt() == 0) 
                {
                    loc_var1->SetUInt(it.first & 0xFFFFFFFF);
                    loc_var2->SetUInt((it.first >> 32) & 0xFFFFFFFF);
                    LOG("Handle of {} set to 0x{:016X} = 0x{:08X} + 0x{:08X} ; Wearer = {}",a_id->GetName(),it.first,loc_var1->GetUInt(),loc_var2->GetUInt(),_cache[it.first].wearer->GetName())
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
        LOG("Could not find script for device {:08X}",a_device->GetFormID())
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
            ERROR("Device {} have null script object",cached.id->GetName())
            continue;
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
                    _cache[loc_handle].wearer   = GetScriptVariable<RE::Actor>(loc_object,"Wearer",RE::FormType::ActorCharacter);

                    LOG("Handle of {} set to 0x{:016X} = 0x{:08X} + 0x{:08X} - Wearer = {}",loc_id->GetName(),a_script.first,loc_var1->GetUInt(),loc_var2->GetUInt(),_cache[loc_handle].wearer ? _cache[loc_handle].wearer->GetName() : "NONE")
                }
            }
        }
    });
    loc_vm->attachedScriptsLock.Unlock();

    DEBUG("Cache size: {}",_cache.size())
    for(auto&& it : _cache) DEBUG("0x{:016X} - {}(0x{:08X}) / 0x{:08X} = {} ; Wearer = {}",it.first,it.second.id->GetName(),it.second.id->GetFormID(),it.second.rd ? it.second.rd->GetFormID() : 0x0,it.second.object->type->name,it.second.wearer ? it.second.wearer->GetName() : "NONE")

    ValidateCache();
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
