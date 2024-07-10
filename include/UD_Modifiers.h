#pragma once

namespace UD
{
    class ModifierManager
    {
    SINGLETONHEADER(ModifierManager)
    public:
        void Setup(){};
        int                         GetModifierIndex(uint32_t a_vm1, uint32_t a_vm2, RE::TESObjectARMO* a_device, std::string a_modifier);
        RE::BGSBaseAlias*           GetModifier(std::string a_modifier);
        std::vector<std::string>    GetModifierStringParamAll(uint32_t a_vm1, uint32_t a_vm2, RE::TESObjectARMO* a_device, std::string a_modifier);
        std::string                 GetModifierStringParam(uint32_t a_vm1, uint32_t a_vm2, RE::TESObjectARMO* a_device, std::string a_modifier);
        bool                        EditModifierStringParam(uint32_t a_vm1, uint32_t a_vm2, RE::TESObjectARMO* a_device, std::string a_modifier, int a_index, std::string a_newvalue);
        std::vector<std::string>    GetModifierAliases(uint32_t a_vm1, uint32_t a_vm2, RE::TESObjectARMO* a_device);
    private:
    };

    // === Utility
    int                         GetStringParamInt(PAPYRUSFUNCHANDLE, std::string a_DataStr,int a_Index,int a_DefaultValue);
    float                       GetStringParamFloat(PAPYRUSFUNCHANDLE, std::string a_DataStr,int a_Index,float a_DefaultValue);
    std::string                 GetStringParamString(PAPYRUSFUNCHANDLE, std::string a_DataStr,int a_Index,std::string a_DefaultValue);
    std::vector<std::string>    GetStringParamAll(PAPYRUSFUNCHANDLE,std::string a_param);
    // === Mods
    int                         GetModifierIndex(PAPYRUSFUNCHANDLE, uint32_t a_vm1, uint32_t a_vm2, RE::TESObjectARMO* a_device, std::string a_modifier);
    RE::BGSBaseAlias*           GetModifier(PAPYRUSFUNCHANDLE, std::string a_modifier);
    std::vector<std::string>    GetModifierStringParamAll(PAPYRUSFUNCHANDLE, uint32_t a_vm1, uint32_t a_vm2, RE::TESObjectARMO* a_device, std::string a_modifier);
    std::string                 GetModifierStringParam(PAPYRUSFUNCHANDLE, uint32_t a_vm1, uint32_t a_vm2, RE::TESObjectARMO* a_device, std::string a_modifier);
    bool                        EditModifierStringParam(PAPYRUSFUNCHANDLE, uint32_t a_vm1, uint32_t a_vm2, RE::TESObjectARMO* a_device, std::string a_modifier, int a_index, std::string a_newvalue);
    std::vector<std::string>    GetModifierAliases(PAPYRUSFUNCHANDLE, uint32_t a_vm1, uint32_t a_vm2, RE::TESObjectARMO* a_device);
}
