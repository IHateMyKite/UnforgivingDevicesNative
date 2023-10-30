#include <UD_PapyrusDelegate.h>

SINGLETONBODY(UD::PapyrusDelegate)

void UD::PapyrusDelegate::Setup(CONFIGFILEARG(a_ptree))
{
    if (!_installed)
    {
        _udrdkw = reinterpret_cast<RE::BGSKeyword*>(RE::TESDataHandler::GetSingleton()->LookupForm(0x11A352,"UnforgivingDevices.esp"));
        _installed = true;
    }
}

int UD::PapyrusDelegate::SendRegisterDeviceScriptEvent(RE::Actor* a_actor, std::vector<RE::TESObjectARMO*>& a_devices)
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

RE::BSScript::ObjectTypeInfo* UD::PapyrusDelegate::IsUnforgivingDevice(RE::BSTSmallSharedArray<RE::BSScript::Internal::AttachedScript>& a_scripts)
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

UD::FilterDeviceResult UD::PapyrusDelegate::CheckRegisterDevice(RE::VMHandle a_handle,RE::BSScript::ObjectTypeInfo* a_type,RE::Actor* a_actor, std::vector<RE::TESObjectARMO*>& a_devices)
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
        const auto loc_wearer = loc_object->GetVariable("Wearer")->GetObject()->Resolve((RE::VMTypeID)RE::FormType::ActorCharacter);
    
        //check if device wearer is the same one as passed actor
        if (loc_wearer == a_actor)
        {
            //get render device from script
            auto loc_rd = (RE::TESObjectARMO*)loc_object->GetProperty("DeviceRendered")->GetObject()->Resolve((RE::VMTypeID)RE::FormType::Armor);
            if (loc_rd != nullptr)
            {
                //check if current device is one in passed array
                if (std::find(a_devices.begin(),a_devices.end(),loc_rd) != a_devices.end())
                {
                    //get inventory device from papyrus script
                    auto loc_id = (RE::TESObjectARMO*)loc_object->GetProperty("DeviceInventory")->GetObject()->Resolve((RE::VMTypeID)RE::FormType::Armor);
                    
                    UDSKSELOG("Device {} found",loc_id ? ((RE::TESObjectARMO*)loc_id)->GetName() : "NONE")
                    
                    //ready function args
                    auto loc_args = new RE::BSScript::FunctionArguments<void, RE::Actor*, RE::TESObjectARMO*, RE::TESObjectARMO*>(std::forward<RE::Actor*>(a_actor),std::forward<RE::TESObjectARMO*>(loc_id),std::forward<RE::TESObjectARMO*>(loc_rd));//RE::MakeFunctionArguments<RE::Actor*,RE::TESObjectARMO*>(a_actor,a_device);
                    
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
