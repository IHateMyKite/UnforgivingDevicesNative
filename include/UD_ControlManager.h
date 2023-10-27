#pragma once

namespace UD
{
    class KeyEventSink : public RE::BSTEventSink<RE::InputEvent*>
    {
    SINGLETONHEADER(KeyEventSink)
    public:
        RE::BSEventNotifyControl ProcessEvent(RE::InputEvent* const* eventPtr,RE::BSTEventSource<RE::InputEvent*>*);

    private:
        std::atomic_bool _Cooldown = false;
        std::atomic_bool _isBound  = false;
    };

    class ControlManager
    {
    SINGLETONHEADER(ControlManager)
    public:
        void Setup();

        void UpdateControl();
        void SyncSetting(bool a_hardcoreMode);

        bool HardcoreMode() const;
        bool PlayerIsBound() const;
        bool PlayerInMinigame() const;
        bool PlayerInZadAnimation() const;

        void CheckStatusSafe(bool* a_result);

        void DebugPrintControls();

        bool HardcoreButtonPressed(uint32_t a_dxkeycode, RE::INPUT_DEVICE a_device);
    private:
        
    private:
        bool _installed = false;

        std::vector<RE::BGSKeyword*> _boundkeywords;
        RE::TESFaction*              _minigamefaction;
        RE::TESFaction*              _animationfaction;
        RE::BSTArray<RE::ControlMap::UserEventMapping>* _OriginalControls;
        RE::BSTArray<RE::ControlMap::UserEventMapping>* _HardcoreControls;
        RE::BSTArray<RE::ControlMap::UserEventMapping>* _DisabledControls;

        //can be found in clibs UserEvents.h

        const std::vector<RE::BSFixedString> _hardcoreids = 
        {
            "Tween Menu",
            "Quick Inventory",
            "Favorites"
        };

        const std::vector<RE::BSFixedString> _disableids = 
        {
            //base player controls
            "Forward"               ,
            "Back"                  ,
            "Strafe Right"          ,
            "Strafe Left"           ,
            "Move"                  ,
            "Activate"              ,
            "Left Attack/Block"     ,
            "Right Attack/Block"    ,
            "Dual Attack"           ,
            "ForceRelease"          ,
            //"Pause"               ,
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
            "MapLookMode",
        };

        bool _hardcoreMode = false;
        bool _ControlsDisabled = false;

        void SaveOriginalControls();
        void DebugPrintControls(RE::BSTArray<RE::ControlMap::UserEventMapping>* a_controls);
        void ApplyControls(RE::BSTArray<RE::ControlMap::UserEventMapping>* a_controls);
    };

    inline void SyncControlSetting(PAPYRUSFUNCHANDLE, bool a_hardcoremode)
    {
        UDSKSELOG("SyncControlSetting({}) called",a_hardcoremode)
        ControlManager::GetSingleton()->SyncSetting(a_hardcoremode);
    }
}