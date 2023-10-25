#pragma once

namespace UD
{

    struct ControlDisable
    {
        RE::ControlMap::UserEventMapping*   ptr;
        RE::ControlMap::UserEventMapping    originalval;
        RE::INPUT_DEVICES::INPUT_DEVICE     device;
    };


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

        const std::vector<ControlDisable>& GetHardcoreControls() const;

        void SyncSetting(bool a_hardcoreMode);

        bool HardcoreMode() const;
        bool PlayerIsBound() const;
        bool PlayerInMinigame() const;
        bool PlayerInZadAnimation() const;
    private:
        
    private:
        bool _installed = false;

        std::vector<RE::BGSKeyword*> _boundkeywords;
        RE::TESFaction*              _minigamefaction;
        RE::TESFaction*              _animationfaction;

        //can be found in clibs UserEvents.h

        const std::vector<RE::BSFixedString> _hardcoreids = 
        {
            "Tween Menu",
            "Quick Inventory",
            "Favorites"
        };

        const std::vector<RE::BSFixedString> _minigameids = 
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

        std::vector<ControlDisable> _hardcorecontrols;
        std::vector<ControlDisable> _minigamecontrols;

        bool _hardcoreMode = false;

        bool _ControlsDisabled = false;

    };

    inline void SyncControlSetting(PAPYRUSFUNCHANDLE, bool a_hardcoremode)
    {
        UDSKSELOG("SyncControlSetting({}) called",a_hardcoremode)
        ControlManager::GetSingleton()->SyncSetting(a_hardcoremode);
    }
}