#pragma once

namespace UD
{
    #define UDBASESCRIPT "ud_customdevice_renderscript"

    using InternalVM = RE::BSScript::Internal::VirtualMachine;



    struct FilterDeviceResult
    {
        bool Result;
        RE::TESObjectARMO* id;
        RE::TESObjectARMO* rd;
    };

    class PapyrusDelegate
    {
    SINGLETONHEADER(PapyrusDelegate)
    public:
        void Setup(CONFIGFILEARG(a_ptree));

        int  SendRegisterDeviceScriptEvent(RE::Actor* a_actor,std::vector<RE::TESObjectARMO*>& a_devices);
    private:
        RE::BSScript::ObjectTypeInfo* IsUnforgivingDevice(RE::BSTSmallSharedArray<RE::BSScript::Internal::AttachedScript>& a_scripts);
        FilterDeviceResult CheckRegisterDevice(RE::VMHandle a_handle,RE::BSScript::ObjectTypeInfo* a_type,RE::Actor* a_actor, std::vector<RE::TESObjectARMO*>& a_devices);
    private:
        bool _installed = false;
        RE::BGSKeyword* _udrdkw;
    };

    inline int SendRegisterDeviceScriptEvent(PAPYRUSFUNCHANDLE,RE::Actor* a_actor,std::vector<RE::TESObjectARMO*> a_devices)
    {
        return PapyrusDelegate::GetSingleton()->SendRegisterDeviceScriptEvent(a_actor,a_devices);
    };
}