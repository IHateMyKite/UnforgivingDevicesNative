#pragma once

namespace UD
{
    //Device mod prototype
    struct DeviceModPrototype
    {
        std::string name;
        uint8_t     group_TES4[32];
        uint8_t     group_ARMO[32];
        size_t      size;
        uint8_t*    rawdata = nullptr;
        std::vector<uint8_t[16]>    devicerecords;
        std::vector<std::string>    masters;
    };

    //Device unit prototype
    struct DeviceUnitPrototype
    {
        std::string scriptName;

        RE::BGSKeyword* kwd                         = nullptr;

        RE::BGSMessage* equipMenu                   = nullptr;
        RE::BGSMessage* zad_DD_OnPutOnDevice        = nullptr;
        RE::BGSMessage* zad_EquipRequiredFailMsg    = nullptr;
        RE::BGSMessage* zad_EquipConflictFailMsg    = nullptr;

        std::vector<RE::BGSKeyword*> equipConflictingDeviceKwds;
        std::vector<RE::BGSKeyword*> requiredDeviceKwds;
        std::vector<RE::BGSKeyword*> unequipConflictingDeviceKwds;
          
        bool lockable;
        bool canManipulate;

        RE::TESObjectARMO*              deviceInventory = nullptr;
        RE::TESObjectARMO*              deviceRendered  = nullptr;

        //following values are set to last values found on last mod (so last overriding mod)
        uint8_t                                 padd_A[16]; //device handle with raw data
        std::vector<RE::BGSKeyword*>            keywords;   //array of keywords loaded from esp - uses keywords from last loaded mod
        std::shared_ptr<DeviceModPrototype>     deviceMod;  //device source mod
            
        //stack of changes by mods. Last record => previous 3 values. First record => original mod record before changes from other mods
        struct HistoryRecord
        {
            std::shared_ptr<DeviceModPrototype> deviceMod;
            uint8_t padd_CB[16];
            std::vector<RE::BGSKeyword*>    keywords;
        };
        std::vector<HistoryRecord>      history; //history stack
    };

    class Diagnosis
    {
    SINGLETONHEADER(Diagnosis)
    public:
        void Setup();

        int CheckPatchedDevices();
    private:
        bool _installed = false;
        bool _imported  = false;
        RE::BGSKeyword* _udid;
        RE::BGSKeyword* _udrd;
    };

    inline int CheckPatchedDevices(PAPYRUSFUNCHANDLE)
    {
        return Diagnosis::GetSingleton()->CheckPatchedDevices();
    }
}