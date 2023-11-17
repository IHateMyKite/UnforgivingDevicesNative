#include <UD_PapyrusDelegate.h>

#undef GetObject

SINGLETONBODY(UD::PapyrusDelegate)

using UD::PapyrusDelegate;
typedef PapyrusDelegate::Result Result;
typedef PapyrusDelegate::FilterDeviceResult FilterDeviceResult;

void PapyrusDelegate::Setup(CONFIGFILEARG(a_ptree))
{
    if (!_installed)
    {
        _udrdkw = reinterpret_cast<RE::BGSKeyword*>(RE::TESDataHandler::GetSingleton()->LookupForm(0x11A352,"UnforgivingDevices.esp"));
        _installed = true;
        UDSKSELOG("PapyrusDelegate::Setup - installed")
    }
}

int PapyrusDelegate::SendRegisterDeviceScriptEvent(RE::Actor* a_actor, std::vector<RE::TESObjectARMO*>& a_devices)
{
    UDSKSELOG("SendRegisterDeviceScriptEvent called")

    if (!_udrdkw) 
    {
        UDSKSELOG("ERROR: UD RD keyword is none !")
        return 0;
    }
    std::erase_if(a_devices,[this](const RE::TESObjectARMO* a_device)
    {
        if (!a_device->HasKeyword(_udrdkw)) return true;
        return false;
    });

    size_t loc_tofound = a_devices.size();

    UDSKSELOG("Finding scripts for {} devices",loc_tofound)

    const auto loc_vm = InternalVM::GetSingleton();
    for (auto&& it : loc_vm->attachedScripts)
    {
        if (loc_tofound == 0) return static_cast<int>(a_devices.size());

        auto loc_type = IsUnforgivingDevice(it.second);
        if (loc_type != nullptr)
        {
            FilterDeviceResult loc_filterres = CheckRegisterDevice(it.first,loc_type,a_actor,a_devices);
            if (loc_filterres.Result) loc_tofound--;
        }
    }

    //some device was not found
    return  static_cast<int>(a_devices.size() - loc_tofound);
}

Result PapyrusDelegate::SendMinigameThreadEvents(RE::Actor* a_actor, RE::TESObjectARMO* a_device, int a_handle1, int a_handle2,MinigameThreads a_threads)
{
    UDSKSELOG("SendMinigameThreadEvents called")
    if (a_actor == nullptr || a_device == nullptr || a_threads == 0) return Result::rArgError;
    if (!a_device->HasKeyword(_udrdkw)) return Result::rDeviceError;

    std::vector<RE::TESObjectARMO*> loc_device = {a_device};
    RE::VMHandle loc_vmhandle = (int64_t)a_handle1 | ((int64_t)a_handle2 << 32);

    const auto loc_vm = InternalVM::GetSingleton();

    for (auto&& it : loc_vm->attachedScripts)
    {
        auto loc_type = IsUnforgivingDevice(it.second);
        if (loc_type != nullptr)
        {
            FilterDeviceResult loc_filterres = ProcessDevice(it.first,loc_vmhandle,loc_type,a_actor,loc_device,[&](RE::BSTSmartPointer<RE::BSScript::Object> a_object,RE::TESObjectARMO* a_id,RE::TESObjectARMO* a_rd)
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
                UDSKSELOG("PapyrusDelegate::SendMinigameThreadEvents - events sent")
            });
            if (loc_filterres.Result) return Result::rSuccess;
        }
    }
    return Result::rNotFound;
}

Result PapyrusDelegate::SendRemoveRenderDeviceEvent(RE::Actor* a_actor, RE::TESObjectARMO* a_device)
{
    UDSKSELOG("SendRemoveRenderDeviceEvent called")
    if (a_actor == nullptr || a_device == nullptr) return Result::rArgError;
    if (!a_device->HasKeyword(_udrdkw)) return Result::rDeviceError;

    std::vector<RE::TESObjectARMO*> loc_device = {a_device};

    const auto loc_vm = InternalVM::GetSingleton();

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
                UDSKSELOG("PapyrusDelegate::SendRemoveRenderDeviceEvent - event sent")
            });
            if (loc_filterres.Result) return Result::rSuccess;
        }
    }
    return Result::rNotFound;
}

Result UD::PapyrusDelegate::SetVMHandle(RE::Actor* a_actor, RE::TESObjectARMO* a_device)
{
    UDSKSELOG("SetVMHandle called")
    if (a_actor == nullptr || a_device == nullptr) return Result::rArgError;
    if (!a_device->HasKeyword(_udrdkw)) return Result::rDeviceError;

    std::vector<RE::TESObjectARMO*> loc_device = {a_device};

    const auto loc_vm = InternalVM::GetSingleton();

    for (auto&& it : loc_vm->attachedScripts)
    {
        auto loc_type = IsUnforgivingDevice(it.second);
        if (loc_type != nullptr)
        {
            bool loc_set = false;
            FilterDeviceResult loc_filterres = ProcessDevice(it.first,0,loc_type,a_actor,loc_device,[&](RE::BSTSmartPointer<RE::BSScript::Object> a_object,RE::TESObjectARMO* a_id,RE::TESObjectARMO* a_rd)
            {
                if (a_object == nullptr) return;

                const auto loc_var1 = a_object->GetVariable("_VMHandle1");
                const auto loc_var2 = a_object->GetVariable("_VMHandle2");

                if (loc_var1 == nullptr || loc_var2 == nullptr) return;

                if (loc_var1->GetSInt() == 0 && loc_var2->GetSInt() == 0) 
                {
                    loc_var1->SetSInt(it.first & 0xFFFFFFFF);
                    loc_var2->SetSInt((it.first >> 32) & 0xFFFFFFFF);
                    UDSKSELOG("Handle set to {} = {} | {}",it.first,loc_var1->GetSInt(),loc_var2->GetSInt())
                    loc_set = true;
                }
            });
            if (loc_set) return Result::rSuccess;
        }
    }
    return Result::rNotFound;
}

RE::BSScript::ObjectTypeInfo* PapyrusDelegate::IsUnforgivingDevice(RE::BSTSmallSharedArray<RE::BSScript::Internal::AttachedScript>& a_scripts)
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
            RE::TESObjectARMO* loc_rd = GetScriptProperty<RE::TESObjectARMO>(loc_object,"DeviceRendered",RE::FormType::Armor);
            if (loc_rd != nullptr)
            {
                //check if current device is one in passed array
                if (std::find(a_devices.begin(),a_devices.end(),loc_rd) != a_devices.end())
                {
                    //get inventory device from papyrus script
                    RE::TESObjectARMO* loc_id = GetScriptProperty<RE::TESObjectARMO>(loc_object,"DeviceInventory",RE::FormType::Armor);
                    
                    UDSKSELOG("Device {} found",loc_id ? ((RE::TESObjectARMO*)loc_id)->GetName() : "NONE")
                    
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
        const RE::Actor* loc_wearer = GetScriptVariable<RE::Actor>(loc_object,"Wearer",RE::FormType::ActorCharacter);//loc_object->GetVariable("Wearer")->GetObject()->Resolve((RE::VMTypeID)RE::FormType::ActorCharacter);
    
        //check if device wearer is the same one as passed actor
        if (loc_wearer == a_actor)
        {
            //get render device from script
            RE::TESObjectARMO* loc_rd = GetScriptProperty<RE::TESObjectARMO>(loc_object,"DeviceRendered",RE::FormType::Armor);//(RE::TESObjectARMO*)loc_object->GetProperty("DeviceRendered")->GetObject()->Resolve((RE::VMTypeID)RE::FormType::Armor);
            if (loc_rd != nullptr)
            {
                //check if current device is one in passed array
                if (std::find(a_devices.begin(),a_devices.end(),loc_rd) != a_devices.end())
                {
                    //get inventory device from papyrus script
                    auto loc_id = GetScriptProperty<RE::TESObjectARMO>(loc_object,"DeviceInventory",RE::FormType::Armor);//(RE::TESObjectARMO*)loc_object->GetProperty("DeviceInventory")->GetObject()->Resolve((RE::VMTypeID)RE::FormType::Armor);
                    
                    if (loc_id != nullptr)
                    {
                        UDSKSELOG("Device {} found",loc_id ? ((RE::TESObjectARMO*)loc_id)->GetName() : "NONE")
                    
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

template<class T>
T* PapyrusDelegate::GetScriptVariable(RE::BSTSmartPointer<RE::BSScript::Object> a_scriptobject, RE::BSFixedString a_variable, RE::FormType a_type)
{
    if (a_scriptobject == nullptr) return nullptr;

    const auto loc_var = a_scriptobject->GetVariable(a_variable);

    if (loc_var == nullptr) return nullptr;

    const auto loc_varobj = loc_var->GetObject();

    if (loc_varobj == nullptr) return nullptr;

    return static_cast<T*>(loc_varobj->Resolve((RE::VMTypeID)a_type));
}

template<class T>
T* PapyrusDelegate::GetScriptProperty(RE::BSTSmartPointer<RE::BSScript::Object> a_scriptobject, RE::BSFixedString a_property, RE::FormType a_type)
{
    if (a_scriptobject == nullptr) return nullptr;

    const auto loc_var = a_scriptobject->GetProperty(a_property);

    if (loc_var == nullptr) return nullptr;

    const auto loc_varobj = loc_var->GetObject();

    if (loc_varobj == nullptr) return nullptr;

    return static_cast<T*>(loc_varobj->Resolve((RE::VMTypeID)a_type));
}
