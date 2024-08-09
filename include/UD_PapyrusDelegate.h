#pragma once

#include <UD_Spinlock.h>

namespace UD
{
    using InternalVM = RE::BSScript::Internal::VirtualMachine;
    using Script = RE::BSTTuple<const RE::VMHandle, RE::BSTSmallSharedArray<RE::BSScript::Internal::AttachedScript>>;

    struct Device
    {
        RE::BSTSmartPointer<RE::BSScript::Object> object = nullptr;
        RE::TESObjectARMO* id       = nullptr;
        RE::TESObjectARMO* rd       = nullptr;
        uint32_t           wearer   = 0U;
    };

    struct Modifier
    {
        RE::BSTSmartPointer<RE::BSScript::Object> object = nullptr;
        std::string name        = "ERROR";
        std::string namealias   = "ERROR";
        RE::TESQuest* quest     = nullptr;
        RE::BGSBaseAlias* alias = nullptr;
    };

    struct ModScript
    {
        std::string scriptname;
        std::string modname;
        RE::FormID formid;
    };
    struct ModScriptVM
    {
        RE::VMHandle handle;
        RE::BSTSmartPointer<RE::BSScript::Object> object;
    };

    typedef RE::TESObjectARMO*(* GetDeviceRender)(RE::TESObjectARMO*);

    class PapyrusDelegate
    {
    SINGLETONHEADER(PapyrusDelegate)
    public:
        enum Result : uint8_t
        {
            rSuccess        = 0,
            rArgError       = 1,
            rDeviceError    = 2,
            rNotFound       = 3
        };

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

        static RE::VMHandle ToVMHandle(const int a_1, const int a_2);

        void Setup();
        void Reload();

        int SendRegisterDeviceScriptEvent(RE::Actor* a_actor,std::vector<RE::TESObjectARMO*>& a_devices);
        Result SendMinigameThreadEvents(RE::Actor* a_actor,RE::TESObjectARMO* a_device,RE::VMHandle a_handle,MinigameThreads a_threads);
        Result SendRemoveRenderDeviceEvent(RE::Actor* a_actor,RE::TESObjectARMO* a_device);
        Result SetBitMapData(RE::VMHandle a_handle,RE::TESObjectARMO* a_device,std::string a_name,int a_val,uint8_t a_size,uint8_t a_off);
        void UpdateVMHandles() const;
        Device GetDeviceScript(int a_handle1,int a_handle2,RE::TESObjectARMO* a_device);
        Device GetCachedDevice(RE::VMHandle,RE::Actor* a_actor, RE::TESObjectARMO* a_device);

        const std::unordered_map<RE::VMHandle,Modifier>& GetModifiers() const;
        std::vector<Modifier> GetModifiers(RE::VMHandle a_handle, RE::TESObjectARMO* a_device);
        Modifier GetModifier(RE::BGSBaseAlias* a_alias) const;
        Modifier GetModifier(std::string a_namealias) const;

        std::vector<RE::BGSBaseAlias*> GetDeviceAliasArray(RE::VMHandle a_handle, RE::TESObjectARMO* a_device, std::string a_pname);
        template<class T> std::vector<T*> GetDeviceFormArray(RE::VMHandle a_handle, RE::TESObjectARMO* a_device, std::string a_pname, RE::FormType a_type);
        std::vector<std::string> GetDeviceStringArray(RE::VMHandle a_handle, RE::TESObjectARMO* a_device, std::string a_pname);

        RE::BSTSmartPointer<RE::BSScript::Object> GetScript(std::string a_script);
        bool GetScriptVariableBool(std::string a_script,std::string a_variable,bool a_property);

        void Lock();
        void Unlock();
    private:
        RE::VMHandle ValidateVMHandle(RE::VMHandle a_handle,RE::TESObjectARMO* a_device);
        void ValidateCache() const;
        RE::BSScript::ObjectTypeInfo* HaveScriptBase(RE::BSTSmallSharedArray<RE::BSScript::Internal::AttachedScript>& a_scripts, const std::string& a_base) const;
        FilterDeviceResult CheckRegisterDevice(RE::VMHandle a_handle,RE::BSScript::ObjectTypeInfo* a_type,RE::Actor* a_actor, std::vector<RE::TESObjectARMO*>& a_devices);
        FilterDeviceResult ProcessDevice(RE::VMHandle a_handle,RE::VMHandle a_handle2,RE::BSScript::ObjectTypeInfo* a_type,RE::Actor* a_actor, std::vector<RE::TESObjectARMO*>& a_devices,std::function<void(RE::BSTSmartPointer<RE::BSScript::Object>,RE::TESObjectARMO*,RE::TESObjectARMO*)> a_fun);
        FilterDeviceResult ProcessDevice2(RE::VMHandle a_handle,RE::VMHandle a_handle2,RE::BSScript::ObjectTypeInfo* a_type,RE::TESObjectARMO* a_device,std::function<bool(RE::BSTSmartPointer<RE::BSScript::Object>,RE::TESObjectARMO*,RE::TESObjectARMO*)> a_fun);
        void ResetCache();
    private:
        bool _installed = false;
        RE::BGSKeyword* _udrdkw;
        template<class T> T* GetScriptVariable(RE::BSTSmartPointer<RE::BSScript::Object> a_scriptobject, RE::BSFixedString a_variable,RE::FormType a_type) const;
        template<class T> T* GetScriptProperty(RE::BSTSmartPointer<RE::BSScript::Object> a_scriptobject, RE::BSFixedString a_property,RE::FormType a_type) const;
        template<class T> std::vector<T*> GetScriptFormArray(RE::BSTSmartPointer<RE::BSScript::Object> a_scriptobject, RE::BSFixedString a_property,RE::FormType a_type);

        mutable uint64_t _RemovedCounter = 0x0; //removed devices counter
        mutable std::unordered_map<RE::VMHandle,Device> _cache;
        mutable std::unordered_map<RE::VMHandle,Modifier> _modifiercache;
        mutable Utils::Spinlock _SaveLock;

        std::vector<std::pair<ModScript,ModScriptVM>> _modscripts =
        {
            {{"UDCustomDeviceMain","UnforgivingDevices.esp",0x15E73C},{}},
            {{"UnforgivingDevicesMain","UnforgivingDevices.esp",0x005901},{}},
            {{"UD_Config","UnforgivingDevices.esp",0x005901},{}}
        };
    };

    inline int SendRegisterDeviceScriptEvent(PAPYRUSFUNCHANDLE,RE::Actor* a_actor,std::vector<RE::TESObjectARMO*> a_devices)
    {
        PapyrusDelegate::GetSingleton()->Lock();
        auto loc_res = PapyrusDelegate::GetSingleton()->SendRegisterDeviceScriptEvent(a_actor,a_devices);
        PapyrusDelegate::GetSingleton()->Unlock();
        return loc_res;
    }

    inline int SendMinigameThreadEvents(PAPYRUSFUNCHANDLE,RE::Actor* a_actor,RE::TESObjectARMO* a_device,int a_handle1,int a_handle2, int a_mode)
    {
        PapyrusDelegate::GetSingleton()->Lock();
        auto loc_res = static_cast<int>(PapyrusDelegate::GetSingleton()->SendMinigameThreadEvents(a_actor,a_device,PapyrusDelegate::ToVMHandle(a_handle1,a_handle2),(PapyrusDelegate::MinigameThreads)a_mode));
        PapyrusDelegate::GetSingleton()->Unlock();
        return loc_res;
    }

    inline int SendRemoveRenderDeviceEvent(PAPYRUSFUNCHANDLE,RE::Actor* a_actor,RE::TESObjectARMO* a_device)
    {
        PapyrusDelegate::GetSingleton()->Lock();
        auto loc_res = static_cast<int>(PapyrusDelegate::GetSingleton()->SendRemoveRenderDeviceEvent(a_actor,a_device));
        PapyrusDelegate::GetSingleton()->Unlock();
        return loc_res;
    }

    inline int SetBitMapData(PAPYRUSFUNCHANDLE,int a_handle1,int a_handle2,RE::TESObjectARMO* a_device,std::string a_name,int a_val,int a_size,int a_off)
    {
        PapyrusDelegate::GetSingleton()->Lock();
        const auto loc_handle = PapyrusDelegate::ToVMHandle(a_handle1,a_handle2);
        auto loc_res = static_cast<int>(PapyrusDelegate::GetSingleton()->SetBitMapData(loc_handle,a_device,a_name,a_val,a_size,a_off));
        PapyrusDelegate::GetSingleton()->Unlock();
        return loc_res;
    }

    inline void UpdateVMHandles(PAPYRUSFUNCHANDLE)
    {
        PapyrusDelegate::GetSingleton()->Lock();
        PapyrusDelegate::GetSingleton()->UpdateVMHandles();
        PapyrusDelegate::GetSingleton()->Unlock();
    }

    template<class T>
    std::vector<T*> PapyrusDelegate::GetScriptFormArray(RE::BSTSmartPointer<RE::BSScript::Object> a_scriptobject, RE::BSFixedString a_property, RE::FormType a_type)
    {
        if (a_scriptobject == nullptr) return std::vector<T*>();

        auto loc_var = a_scriptobject->GetProperty(a_property);

        if (loc_var == nullptr) return std::vector<T*>();

        auto loc_vararrptr = loc_var->GetArray();

        if (loc_vararrptr == nullptr) return std::vector<T*>();

        auto loc_vararr = loc_vararrptr.get();

        std::vector<T*> loc_res;
        for (uint8_t i = 0; i < loc_vararr->size(); i++)
        {
            //undef this stupidass macro so we can use the GetObject method
            #undef GetObject
            auto loc_varobj = (*loc_vararr)[i].GetObject();
            loc_res.push_back(static_cast<T*>(loc_varobj->Resolve((RE::VMTypeID)a_type)));

            //for (int a = 0; a < 255; a++) DEBUG("{} -> {}",a,loc_varobj->Resolve((RE::VMTypeID)a))
        }

        return loc_res;
    }


    template<class T> std::vector<T*> PapyrusDelegate::GetDeviceFormArray(RE::VMHandle a_handle, RE::TESObjectARMO* a_device, std::string a_pname, RE::FormType a_type)
    {
        RE::VMHandle loc_vmhandle = a_handle;
        loc_vmhandle = ValidateVMHandle(loc_vmhandle,a_device);

        ValidateCache();
        Device loc_cacheres = _cache[loc_vmhandle];

        auto loc_res = GetScriptPropertyArray<T>(loc_cacheres.object,a_pname,a_type);

        return loc_res;
    }
}