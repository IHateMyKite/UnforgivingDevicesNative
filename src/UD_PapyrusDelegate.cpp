#include <UD_PapyrusDelegate.h>

#undef GetObject

SINGLETONBODY(UD::PapyrusDelegate)

using UD::PapyrusDelegate;
typedef PapyrusDelegate::Result Result;
typedef PapyrusDelegate::FilterDeviceResult FilterDeviceResult;

RE::VMHandle UD::PapyrusDelegate::ToVMHandle(const int a_1, const int a_2)
{
    return (int64_t)a_1 | ((int64_t)a_2 << 32);
}

void PapyrusDelegate::Setup()
{
    if (!_installed)
    {
        _udrdkw = reinterpret_cast<RE::BGSKeyword*>(RE::TESDataHandler::GetSingleton()->LookupForm(0x11A352,"UnforgivingDevices.esp"));
        _installed = true;
        LOG("PapyrusDelegate::Setup - installed")
    }
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
    loc_vm->attachedScriptsLock.Lock();
    for (auto&& it : loc_vm->attachedScripts)
    {
        if (loc_tofound == 0) 
        {
            loc_vm->attachedScriptsLock.Unlock();
            return static_cast<int>(a_devices.size());
        }

        auto loc_type = IsUnforgivingDevice(it.second);
        if (loc_type != nullptr)
        {
            FilterDeviceResult loc_filterres = CheckRegisterDevice(it.first,loc_type,a_actor,a_devices);
            if (loc_filterres.Result) loc_tofound--;
        }
    }
    loc_vm->attachedScriptsLock.Unlock();

    //some device was not found
    return  static_cast<int>(a_devices.size() - loc_tofound);
}

Result PapyrusDelegate::SendMinigameThreadEvents(RE::Actor* a_actor, RE::TESObjectARMO* a_device, RE::VMHandle a_handle,MinigameThreads a_threads)
{
    LOG("SendMinigameThreadEvents called")
    if (a_actor == nullptr || a_device == nullptr || a_threads == 0) return Result::rArgError;
    if (!a_device->HasKeyword(_udrdkw)) return Result::rDeviceError;

    std::vector<RE::TESObjectARMO*> loc_device = {a_device};

    RE::VMHandle loc_vmhandle = a_handle;
    loc_vmhandle = ValidateVMHandle(loc_vmhandle,a_device);

    const auto loc_vm = InternalVM::GetSingleton();

    loc_vm->attachedScriptsLock.Lock();
    auto loc_scripts = loc_vm->attachedScripts.find(loc_vmhandle);
    loc_vm->attachedScriptsLock.Unlock();

    auto loc_type = IsUnforgivingDevice(loc_scripts->second);
    if (loc_type != nullptr)
    {
        FilterDeviceResult loc_filterres = ProcessDevice(loc_scripts->first,loc_vmhandle,loc_type,a_actor,loc_device,[&](RE::BSTSmartPointer<RE::BSScript::Object> a_object,RE::TESObjectARMO* a_id,RE::TESObjectARMO* a_rd)
        {
            //init unused callback
            RE::BSTSmartPointer<RE::BSScript::IStackCallbackFunctor> loc_callback1;
            RE::BSTSmartPointer<RE::BSScript::IStackCallbackFunctor> loc_callback2;
            RE::BSTSmartPointer<RE::BSScript::IStackCallbackFunctor> loc_callback3;
            RE::BSTSmartPointer<RE::BSScript::IStackCallbackFunctor> loc_callback4;
            //call papyrus method
            if (a_threads & tStarter)   loc_vm->DispatchMethodCall(a_object,"_MinigameStarterThread",RE::MakeFunctionArguments(),loc_callback1);
            if (a_threads & tCrit)      loc_vm->DispatchMethodCall(a_object,"_MinigameCritLoopThread",RE::MakeFunctionArguments(),loc_callback2);
            if (a_threads & tParalel)   loc_vm->DispatchMethodCall(a_object,"_MinigameParalelThread",RE::MakeFunctionArguments(),loc_callback3);
            if (a_threads & tAV)        loc_vm->DispatchMethodCall(a_object,"_MinigameAVCheckLoopThread",RE::MakeFunctionArguments(),loc_callback4);
            LOG("PapyrusDelegate::SendMinigameThreadEvents - events sent")
        });
        if (loc_filterres.Result) return Result::rSuccess;
    }
    return Result::rNotFound;
}

Result PapyrusDelegate::SendRemoveRenderDeviceEvent(RE::Actor* a_actor, RE::TESObjectARMO* a_device)
{
    LOG("SendRemoveRenderDeviceEvent called")
    if (a_actor == nullptr || a_device == nullptr) return Result::rArgError;
    if (!a_device->HasKeyword(_udrdkw)) return Result::rDeviceError;

    std::vector<RE::TESObjectARMO*> loc_device = {a_device};

    const auto loc_vm = InternalVM::GetSingleton();
    loc_vm->attachedScriptsLock.Lock();

    for (auto&& it : loc_vm->attachedScripts)
    {
        auto loc_type = IsUnforgivingDevice(it.second);
        if (loc_type != nullptr)
        {
            FilterDeviceResult loc_filterres = ProcessDevice(it.first,0,loc_type,a_actor,loc_device,[&](RE::BSTSmartPointer<RE::BSScript::Object> a_object,RE::TESObjectARMO* a_id,RE::TESObjectARMO* a_rd)
            {
                //init unused callback
                RE::BSTSmartPointer<RE::BSScript::IStackCallbackFunctor> loc_callback;
                auto loc_args = new RE::BSScript::FunctionArguments<void, RE::Actor*>(std::forward<RE::Actor*>(a_actor));
                //call papyrus method
                loc_vm->DispatchMethodCall(a_object,"removeDevice",loc_args,loc_callback);
                LOG("PapyrusDelegate::SendRemoveRenderDeviceEvent - event sent")
            });
            if (loc_filterres.Result) 
            {
                loc_vm->attachedScriptsLock.Unlock();
                return Result::rSuccess;
            }
        }
    }
    loc_vm->attachedScriptsLock.Unlock();
    return Result::rNotFound;
}

Result UD::PapyrusDelegate::SetBitMapData(RE::VMHandle a_handle, RE::TESObjectARMO* a_device, std::string a_name, int a_val, uint8_t a_size, uint8_t a_off)
{
    //LOG("SetBitMapData called")
    if (a_name == "" || a_size > 32 || a_off > 32 || a_device == nullptr) return Result::rArgError;
    if (!a_device->HasKeyword(_udrdkw)) return Result::rDeviceError;

    RE::VMHandle loc_vmhandle = a_handle;
    loc_vmhandle = ValidateVMHandle(loc_vmhandle,a_device);
    const auto loc_vm = InternalVM::GetSingleton();

    RE::BSScript::ObjectTypeInfo* loc_type = nullptr;

    bool loc_found = false;
    loc_vm->attachedScriptsLock.Lock();
    auto loc_scripts = loc_vm->attachedScripts.find(loc_vmhandle);
    loc_found = (loc_scripts != loc_vm->attachedScripts.end());
    loc_vm->attachedScriptsLock.Unlock();
    
    if (loc_found)
    {
        loc_type = IsUnforgivingDevice(loc_scripts->second);
        if (loc_type != nullptr)
        {
            FilterDeviceResult loc_filterres = ProcessDevice2(loc_scripts->first,loc_vmhandle,loc_type,a_device,[&](RE::BSTSmartPointer<RE::BSScript::Object> a_object,RE::TESObjectARMO* a_id,RE::TESObjectARMO* a_rd) -> bool
            {
                if (a_object == nullptr)
                {
                    ERROR("Found object is none")
                    return false;
                }
                const auto loc_var = a_object->GetVariable(a_name);

                if (loc_var == nullptr)
                {
                    ERROR("Could not find bitmap variable {}",a_name)
                    return false;
                }
                int loc_res = Utility::GetSingleton()->CodeBit(loc_var->GetSInt(),a_val,a_size,a_off);
                loc_var->SetSInt(loc_res);
            
                return true;
            });
        
            if (loc_filterres.Result) return Result::rSuccess;
        }
    }
    else
    {
        LOG("Script not found in attached script. Searching scripts for cleanup - {}",a_handle)
        RE::BSTSmartPointer<RE::BSScript::Object> loc_object;
        loc_vm->objectResetLock.Lock();
        auto loc_it = std::find_if(loc_vm->objectsAwaitingCleanup.begin(),loc_vm->objectsAwaitingCleanup.end(),[&loc_object,a_handle,this](RE::BSTSmartPointer<RE::BSScript::Object> a_object)
        {
            auto loc_vmho1 = a_object->GetVariable("_VMHandle1");
            auto loc_vmho2 = a_object->GetVariable("_VMHandle2");
            if (loc_vmho1 == nullptr || loc_vmho2 == nullptr) return false;
            auto loc_vmh1 = loc_vmho1->GetSInt();
            auto loc_vmh2 = loc_vmho2->GetSInt();
            auto loc_vmh = ToVMHandle(loc_vmh1,loc_vmh2);
            LOG("VMHandles found {}",loc_vmh)
            if (loc_vmh == a_handle)
            {
                loc_object = a_object;
                return true;
            }
            else return false;
        });

        loc_found = loc_it != loc_vm->objectsAwaitingCleanup.end();
        loc_vm->objectResetLock.Unlock();
        if (loc_found) loc_type = loc_it->get()->type.get();

        if (loc_object == nullptr)
        {
            ERROR("Found object is none")
            return Result::rNotFound;
        }
        const auto loc_var = loc_object->GetVariable(a_name);

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
    //LOG("PapyrusDelegate::ValidateVMHandle called")

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

                if (loc_var1->GetSInt() == 0 && loc_var2->GetSInt() == 0) 
                {
                    loc_var1->SetSInt(it.first & 0xFFFFFFFF);
                    loc_var2->SetSInt((it.first >> 32) & 0xFFFFFFFF);
                    LOG("Handle of {} set to {} = {} | {}",a_id->GetName(),it.first,loc_var1->GetSInt(),loc_var2->GetSInt())
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
    const auto loc_vm = InternalVM::GetSingleton();

    loc_vm->attachedScriptsLock.Lock();
    for (auto&& it : loc_vm->attachedScripts)
    {
        auto loc_type = IsUnforgivingDevice(it.second);
        if (loc_type != nullptr)
        {
            RE::VMHandle loc_handle = it.first;
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

                    if (loc_var1 == nullptr || loc_var2 == nullptr) continue;

                    loc_var1->SetSInt(loc_handle & 0xFFFFFFFF);
                    loc_var2->SetSInt((loc_handle >> 32) & 0xFFFFFFFF);
                    LOG("Handle of {} set to {} = {} | {}",loc_id->GetName(),it.first,loc_var1->GetSInt(),loc_var2->GetSInt())
                }
            }
        }
    }
    loc_vm->attachedScriptsLock.Unlock();
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
