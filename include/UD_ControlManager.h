#pragma once

#include <UD_PapyrusDelegate.h>

namespace UD
{
    #define CONTROLSDISABLE RE::INPUT_DEVICES::kGamepad

    class KeyEventSink : public RE::BSTEventSink<RE::InputEvent*>
    {
    SINGLETONHEADER(KeyEventSink)
    public:
        RE::BSEventNotifyControl ProcessEvent(RE::InputEvent* const* eventPtr,RE::BSTEventSource<RE::InputEvent*>*);

    private:
        bool _Cooldown = false;
    };

    class CameraEventSink : public RE::BSTEventSink<SKSE::CameraEvent>
    {
    SINGLETONHEADER(CameraEventSink)
    public:
        RE::BSEventNotifyControl ProcessEvent(const SKSE::CameraEvent* eventPtr,RE::BSTEventSource<SKSE::CameraEvent>*);
        int GetCameraState() const;
    private:
        RE::TESCameraState* _state = nullptr;
    };

    enum CallbackArgFuns : uint8_t
    {
        fWidgetValue = 0
    };

    enum CallbackArgType : uint8_t
    {
        tInt    = 0,
        tFloat  = 1,
        tString = 2,
        tForm   = 3
    };

    struct DeviceCallbackArg
    {
        CallbackArgFuns funtype;
        CallbackArgType type;
        std::string argStr; 
        RE::TESForm* argForm;
        std::function<void*(std::string a_argStr, RE::TESForm* a_argForm)> fun;
    };

    struct DeviceCallback
    {
        Device device;
        std::string callback;
        std::vector<DeviceCallbackArg> args;
        void Send();
    };

    using RE::BSScript::Variable;

    class VarFunctionArguments : public RE::BSScript::ZeroFunctionArguments
    {
        public:
            // override (IFunctionArguments)
            bool operator()(RE::BSScrapArray<Variable>& a_dst) const;
            void SetArgs(std::vector<DeviceCallbackArg>& a_args)
            {
                _args = a_args;
            }
        private:
            std::vector<DeviceCallbackArg> _args;
    };

    class ControlManager
    {
    SINGLETONHEADER(ControlManager)
    public:
        void Setup();

        void UpdateControl();
        void SyncSetting(bool a_hardcoreMode);

        bool HardcoreMode() const;

        void CheckStatusSafe(bool* a_result);

        void DebugPrintControls();

        bool HardcoreButtonPressed(uint32_t a_dxkeycode, RE::INPUT_DEVICE a_device);

        void ApplyOriginalControls();
        void DisableControls();
        void DisableControlsFC();

        const std::vector<std::string>& GetHardcoreMessages() const;

        bool RegisterDeviceCallback(int a_handle1,int a_handle2,RE::TESObjectARMO* a_device, int a_dxkeycode, std::string a_callbackfun);
        bool AddDeviceCallbackArgument(int a_dxkeycode, CallbackArgFuns a_type, std::string a_argStr, RE::TESForm* a_argForm);
        bool UnregisterDeviceCallbacks(int a_handle1,int a_handle2,RE::TESObjectARMO* a_device);
        void UnregisterAllDeviceCallbacks();
        const std::unordered_map<uint32_t,DeviceCallback>& GetDeviceCallbacks();
        bool HaveDeviceCallbacks() const;
    private:
        void AddArgument(DeviceCallback* a_callback, CallbackArgFuns a_type, std::string a_argStr, RE::TESForm* a_argForm);
        static void DrawWeaponMagicHands(RE::Actor* a_actor, bool a_draw);
        inline static REL::Relocation<decltype(DrawWeaponMagicHands)> DrawWeaponMagicHands_old;
    private:
        bool _installed = false;
        RE::BSTArray<RE::ControlMap::UserEventMapping>* _OriginalControls;
        RE::BSTArray<RE::ControlMap::UserEventMapping>* _HardcoreControls;
        RE::BSTArray<RE::ControlMap::UserEventMapping>* _DisabledControls;
        RE::BSTArray<RE::ControlMap::UserEventMapping>* _DisabledNoMoveControls;

        std::unordered_map<uint32_t,DeviceCallback> _DeviceCallbacks;

        //can be found in clibs UserEvents.h
        std::vector<std::string> _hardcoreids;
        std::vector<std::string> _disableids; 

        std::vector<std::string> _disablenomoveids = 
        {
            //base player controls
            "Activate"              ,
            "Left Attack/Block"     ,
            "Right Attack/Block"    ,
            "Dual Attack"           ,
            "ForceRelease"          ,
            "Ready Weapon"          ,
            "Toggle POV"            ,
            "Jump"                  ,
            "Sprint"                ,
            "Sneak"                 ,
            "Shout"                 ,
            "KinectShout"           ,
            "Grab"                  ,
            "Run"                   ,
            "Toggle Always Run"     ,
            "Auto-Move"             ,

            //menu control
            "Journal",
            "Tween Menu",
            "Quick Inventory",
            "Quick Magic",
            "Quick Map",
            "Quick Stats",
            "Wait",
            "Favorites",
            "Hotkey1",
            "Hotkey2",
            "Hotkey3",
            "Hotkey4",
            "Hotkey5",
            "Hotkey6",
            "Hotkey7",
            "Hotkey8",
            "Inventory",
            "LeftEquip",
            "RightEquip",
            "MapLookMode"
        };

        bool _hardcoreMode = false;
        bool _ControlsDisabled = false;
        bool _HardcoreModeApplied = false;

        bool _DisableFreeCamera = true;
        std::vector<std::string> _hardcodemessages;

        void SaveOriginalControls();
        void InitControlOverride(RE::BSTArray<RE::ControlMap::UserEventMapping>** a_controls,const std::vector<std::string>& a_filter);
        void DebugPrintControls(RE::BSTArray<RE::ControlMap::UserEventMapping>* a_controls);
        void ApplyControls(RE::BSTArray<RE::ControlMap::UserEventMapping>* a_controls);
    };

    inline void SyncControlSetting(PAPYRUSFUNCHANDLE, bool a_hardcoremode)
    {
        ControlManager::GetSingleton()->SyncSetting(a_hardcoremode);
    }

    inline int GetCameraState(PAPYRUSFUNCHANDLE)
    {
        return CameraEventSink::GetSingleton()->GetCameraState();
    }

    inline bool RegisterDeviceCallback(PAPYRUSFUNCHANDLE, int a_handle1,int a_handle2,RE::TESObjectARMO* a_device, int a_dxkeycode, std::string a_callbackfun)
    {
        return ControlManager::GetSingleton()->RegisterDeviceCallback(a_handle1,a_handle2,a_device,a_dxkeycode,a_callbackfun);
    }

    inline bool UnregisterDeviceCallbacks(PAPYRUSFUNCHANDLE, int a_handle1,int a_handle2,RE::TESObjectARMO* a_device)
    {
        return ControlManager::GetSingleton()->UnregisterDeviceCallbacks(a_handle1,a_handle2,a_device);
    }

    inline void UnregisterAllDeviceCallbacks(PAPYRUSFUNCHANDLE)
    {
        return ControlManager::GetSingleton()->UnregisterAllDeviceCallbacks();
    }

    inline bool AddDeviceCallbackArgument(PAPYRUSFUNCHANDLE, int a_dxkeycode, int a_type, std::string a_argStr, RE::TESForm* a_argForm)
    {
        return ControlManager::GetSingleton()->AddDeviceCallbackArgument(a_dxkeycode,(CallbackArgFuns)a_type,a_argStr,a_argForm);
    }
}