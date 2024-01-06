#pragma once

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
    private:
        
    private:
        bool _installed = false;
        RE::BSTArray<RE::ControlMap::UserEventMapping>* _OriginalControls;
        RE::BSTArray<RE::ControlMap::UserEventMapping>* _HardcoreControls;
        RE::BSTArray<RE::ControlMap::UserEventMapping>* _DisabledControls;
        RE::BSTArray<RE::ControlMap::UserEventMapping>* _DisabledNoMoveControls;

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
        LOG("SyncControlSetting({}) called",a_hardcoremode)
        ControlManager::GetSingleton()->SyncSetting(a_hardcoremode);
    }

    inline int GetCameraState(PAPYRUSFUNCHANDLE)
    {
        return CameraEventSink::GetSingleton()->GetCameraState();
    }
}