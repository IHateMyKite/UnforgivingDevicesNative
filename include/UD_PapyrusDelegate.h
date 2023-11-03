#pragma once

namespace UD
{
    #define UDBASESCRIPT "ud_customdevice_renderscript"

    using InternalVM = RE::BSScript::Internal::VirtualMachine;

    enum MinigameThreads : uint8_t
    {
        tStarter = 0x01,
        tCrit    = 0x02,
        tParalel = 0x04,
        tAV      = 0x08
    };

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
        void SendMinigameThreadEvents(RE::Actor* a_actor,RE::TESObjectARMO* a_device,MinigameThreads a_threads);
        void SendRemoveRenderDeviceEvent(RE::Actor* a_actor,RE::TESObjectARMO* a_device);

    private:
        RE::BSScript::ObjectTypeInfo* IsUnforgivingDevice(RE::BSTSmallSharedArray<RE::BSScript::Internal::AttachedScript>& a_scripts);
        FilterDeviceResult CheckRegisterDevice(RE::VMHandle a_handle,RE::BSScript::ObjectTypeInfo* a_type,RE::Actor* a_actor, std::vector<RE::TESObjectARMO*>& a_devices);
        FilterDeviceResult ProcessDevice(RE::VMHandle a_handle,RE::BSScript::ObjectTypeInfo* a_type,RE::Actor* a_actor, std::vector<RE::TESObjectARMO*>& a_devices,std::function<void(RE::BSTSmartPointer<RE::BSScript::Object>,RE::TESObjectARMO*,RE::TESObjectARMO*)> a_fun);
    private:
        bool _installed = false;
        RE::BGSKeyword* _udrdkw;
    };

    inline int SendRegisterDeviceScriptEvent(PAPYRUSFUNCHANDLE,RE::Actor* a_actor,std::vector<RE::TESObjectARMO*> a_devices)
    {
        return PapyrusDelegate::GetSingleton()->SendRegisterDeviceScriptEvent(a_actor,a_devices);
    }

    inline void SendMinigameThreadEvents(PAPYRUSFUNCHANDLE,RE::Actor* a_actor,RE::TESObjectARMO* a_device, int a_mode)
    {
        PapyrusDelegate::GetSingleton()->SendMinigameThreadEvents(a_actor,a_device,(MinigameThreads)a_mode);
    }

    inline void SendRemoveRenderDeviceEvent(PAPYRUSFUNCHANDLE,RE::Actor* a_actor,RE::TESObjectARMO* a_device)
    {
        PapyrusDelegate::GetSingleton()->SendRemoveRenderDeviceEvent(a_actor,a_device);
    }
}