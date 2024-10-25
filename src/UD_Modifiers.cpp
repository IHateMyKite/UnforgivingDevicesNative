#include <UD_Modifiers.h>
#include <UD_Macros.h>
#include <UD_Config.h>
#include <UD_Utility.h>
#include <UD_PapyrusDelegate.h>
#include <boost/algorithm/string.hpp>

SINGLETONBODY(UD::ModifierManager)

int UD::GetStringParamInt(PAPYRUSFUNCHANDLE, std::string a_DataStr, int a_Index, int a_DefaultValue)
{
    //LOG("GetStringParamInt({},{},{})",a_DataStr,a_Index,a_DefaultValue)
    return GetStringParam<int>(a_DataStr,a_Index,a_DefaultValue);
}

float UD::GetStringParamFloat(PAPYRUSFUNCHANDLE, std::string a_DataStr, int a_Index, float a_DefaultValue)
{
    //LOG("GetStringParamFloat({},{},{})",a_DataStr,a_Index,a_DefaultValue)
    return GetStringParam<float>(a_DataStr,a_Index,a_DefaultValue);
}

std::string UD::GetStringParamString(PAPYRUSFUNCHANDLE, std::string a_DataStr, int a_Index, std::string a_DefaultValue)
{
    //LOG("GetStringParamString({},{},{})",a_DataStr,a_Index,a_DefaultValue)
    return GetStringParam<std::string>(a_DataStr,a_Index,a_DefaultValue);
}

std::vector<std::string> UD::GetStringParamAll(PAPYRUSFUNCHANDLE, std::string a_param)
{
    //LOG("GetModifierAllParam({})",a_param)
    return GetStringParamAllInter<std::string>(a_param,",");
}

int UD::GetModifierIndex(PAPYRUSFUNCHANDLE, uint32_t a_vm1, uint32_t a_vm2, RE::TESObjectARMO* a_device, std::string a_modifier)
{
    return ModifierManager::GetSingleton()->GetModifierIndex(a_vm1,a_vm2,a_device,a_modifier);
}

RE::BGSBaseAlias* UD::GetModifier(PAPYRUSFUNCHANDLE, std::string a_modifier)
{
    return ModifierManager::GetSingleton()->GetModifier(a_modifier);
}

std::vector<std::string> UD::GetModifierStringParamAll(PAPYRUSFUNCHANDLE, uint32_t a_vm1, uint32_t a_vm2, RE::TESObjectARMO* a_device, std::string a_modifier)
{
    return ModifierManager::GetSingleton()->GetModifierStringParamAll(a_vm1,a_vm2,a_device,a_modifier);
}

std::string UD::GetModifierStringParam(PAPYRUSFUNCHANDLE, uint32_t a_vm1, uint32_t a_vm2, RE::TESObjectARMO* a_device, std::string a_modifier)
{
    return ModifierManager::GetSingleton()->GetModifierStringParam(a_vm1,a_vm2,a_device,a_modifier);
}

bool UD::EditModifierStringParam(PAPYRUSFUNCHANDLE, uint32_t a_vm1, uint32_t a_vm2, RE::TESObjectARMO* a_device, std::string a_modifier, int a_index, std::string a_newvalue)
{
    return ModifierManager::GetSingleton()->EditModifierStringParam(a_vm1,a_vm2,a_device,a_modifier,a_index,a_newvalue);
}

std::vector<std::string> UD::GetModifierAliases(PAPYRUSFUNCHANDLE, uint32_t a_vm1, uint32_t a_vm2, RE::TESObjectARMO* a_device)
{
    return ModifierManager::GetSingleton()->GetModifierAliases(a_vm1,a_vm2,a_device);
}

int UD::ModifierManager::GetModifierIndex(uint32_t a_vm1, uint32_t a_vm2, RE::TESObjectARMO* a_device, std::string a_modifier)
{
    LOG("GetModifierIndex(0x{:016X},0x{:08X},{}) - Called",PapyrusDelegate::GetSingleton()->ToVMHandle(a_vm1,a_vm2),a_device ? a_device->GetFormID() : 0,a_modifier)

    if (a_device == nullptr) return -1;

    auto loc_modifiers = PapyrusDelegate::GetSingleton()->GetModifiers(PapyrusDelegate::GetSingleton()->ToVMHandle(a_vm1,a_vm2),a_device);

    const auto loc_it = std::find_if(loc_modifiers.begin(),loc_modifiers.end(),[&](Modifier& a_mod){return a_mod.namealias == a_modifier;});

    if (loc_it == loc_modifiers.end())
    {
        LOG("GetModifierIndex(0x{:016X},0x{:08X},{}) - Could not find modifier",PapyrusDelegate::GetSingleton()->ToVMHandle(a_vm1,a_vm2),a_device->GetFormID(),a_modifier)
        return -1;
    }

    return loc_it - loc_modifiers.begin();
}

RE::BGSBaseAlias* UD::ModifierManager::GetModifier(std::string a_modifier)
{
    const auto loc_modifiers = UD::PapyrusDelegate::GetSingleton()->GetModifiers();

    for (auto&& [vmhandle, modifier] : loc_modifiers)
    {
        if (modifier.namealias == a_modifier)
        {
            return modifier.alias;

        }
    }

    return nullptr;
}

std::vector<std::string> UD::ModifierManager::GetModifierStringParamAll(uint32_t a_vm1, uint32_t a_vm2, RE::TESObjectARMO* a_device, std::string a_modifier)
{
    LOG("GetModifierStringParamAll(0x{:016X},0x{:08X},{}) - Called",PapyrusDelegate::GetSingleton()->ToVMHandle(a_vm1,a_vm2),a_device ? a_device->GetFormID() : 0,a_modifier)

    if (a_device == nullptr || ((a_vm1 + a_vm2) == 0U)) return std::vector<std::string>();

    auto loc_modifierparam = PapyrusDelegate::GetSingleton()->GetDeviceStringArray(PapyrusDelegate::GetSingleton()->ToVMHandle(a_vm1,a_vm2),a_device,"UD_ModifiersDataStr");
    auto loc_modifiers = PapyrusDelegate::GetSingleton()->GetModifiers(PapyrusDelegate::GetSingleton()->ToVMHandle(a_vm1,a_vm2),a_device);

    const auto loc_it = std::find_if(loc_modifiers.begin(),loc_modifiers.end(),[&](Modifier& a_mod){return a_mod.namealias == a_modifier;});

    if (loc_it == loc_modifiers.end())
    {
        WARN("GetModifierStringParamAll(0x{:016X},0x{:08X},{}) - Could not find modifier",PapyrusDelegate::GetSingleton()->ToVMHandle(a_vm1,a_vm2),a_device->GetFormID(),a_modifier)
        return std::vector<std::string>();
    }

    const std::string loc_param = loc_modifierparam[loc_it - loc_modifiers.begin()];

    auto loc_res = GetStringParamAllInter<std::string>(loc_param,",");

    return loc_res;
}

std::string UD::ModifierManager::GetModifierStringParam(uint32_t a_vm1, uint32_t a_vm2, RE::TESObjectARMO* a_device, std::string a_modifier)
{
    LOG("GetModifierStringParam(0x{:016X},0x{:08X},{}) - Called",PapyrusDelegate::GetSingleton()->ToVMHandle(a_vm1,a_vm2),a_device ? a_device->GetFormID() : 0,a_modifier)

    if (a_device == nullptr || ((a_vm1 + a_vm2) == 0U)) return "";

    auto loc_modifierparam = PapyrusDelegate::GetSingleton()->GetDeviceStringArray(PapyrusDelegate::GetSingleton()->ToVMHandle(a_vm1,a_vm2),a_device,"UD_ModifiersDataStr");
    auto loc_modifiers = PapyrusDelegate::GetSingleton()->GetModifiers(PapyrusDelegate::GetSingleton()->ToVMHandle(a_vm1,a_vm2),a_device);

    const auto loc_it = std::find_if(loc_modifiers.begin(),loc_modifiers.end(),[&](Modifier& a_mod){return a_mod.namealias == a_modifier;});

    if (loc_it == loc_modifiers.end())
    {
        WARN("GetModifierStringParam(0x{:016X},0x{:08X},{}) - Could not find modifier",PapyrusDelegate::GetSingleton()->ToVMHandle(a_vm1,a_vm2),a_device->GetFormID(),a_modifier)
        return "";
    }

    const std::string loc_param = loc_modifierparam[loc_it - loc_modifiers.begin()];

    return loc_param;
}

bool UD::ModifierManager::EditModifierStringParam(uint32_t a_vm1, uint32_t a_vm2, RE::TESObjectARMO* a_device, std::string a_modifier, int a_index, std::string a_newvalue)
{
    LOG("EditModifierStringParam(0x{:016X},0x{:08X},{},{},{}) - Called",PapyrusDelegate::GetSingleton()->ToVMHandle(a_vm1,a_vm2),a_device ? a_device->GetFormID() : 0,a_modifier,a_index,a_newvalue)

    if (a_device == nullptr || ((a_vm1 + a_vm2) == 0U)) return false;

    auto loc_modifierparam = PapyrusDelegate::GetSingleton()->GetDeviceStringArray(PapyrusDelegate::GetSingleton()->ToVMHandle(a_vm1,a_vm2),a_device,"UD_ModifiersDataStr");
    auto loc_modifiers = PapyrusDelegate::GetSingleton()->GetModifiers(PapyrusDelegate::GetSingleton()->ToVMHandle(a_vm1,a_vm2),a_device);

    const auto loc_it = std::find_if(loc_modifiers.begin(),loc_modifiers.end(),[&](Modifier& a_mod){return a_mod.namealias == a_modifier;});

    auto loc_device = PapyrusDelegate::GetSingleton()->GetDeviceScript(a_vm1,a_vm2,a_device);

    if (loc_it == loc_modifiers.end())
    {
        WARN("EditModifierStringParam(0x{:016X},0x{:08X},{},{},{}) - Could not find modifier",PapyrusDelegate::GetSingleton()->ToVMHandle(a_vm1,a_vm2),a_device->GetFormID(),a_modifier,a_index,a_newvalue)
        return false;
    }

    auto loc_obj = loc_device.object->GetProperty("UD_ModifiersDataStr");

    if (loc_obj->IsNoneArray())
    {
        ERROR("EditModifierStringParam(0x{:016X},0x{:08X},{},{},{}) - Device doesnt have parameter list set up. Setting it up...",PapyrusDelegate::GetSingleton()->ToVMHandle(a_vm1,a_vm2),a_device->GetFormID(),a_modifier,a_index,a_newvalue)
            
        RE::BSTSmartPointer<RE::BSScript::Array> a_val;
        const auto loc_vm = InternalVM::GetSingleton();


        loc_vm->CreateArray(RE::BSScript::TypeInfo(RE::BSScript::TypeInfo::RawType::kString), loc_modifiers.size(), a_val);

        loc_obj->SetArray(a_val);
    }
    else if (!loc_obj->IsArray())
    {
        ERROR("EditModifierStringParam(0x{:016X},0x{:08X},{},{},{}) - UD_ModifiersDataStr is not array for some reason",PapyrusDelegate::GetSingleton()->ToVMHandle(a_vm1,a_vm2),a_device->GetFormID(),a_modifier,a_index,a_newvalue)
            
        return false;
    }

    const size_t loc_id = loc_it - loc_modifiers.begin();

    std::string loc_param = loc_modifierparam[loc_id];

    auto loc_params = GetStringParamAllInter<std::string>(loc_param,",");

    // set new value
    loc_params[a_index] = a_newvalue;

    loc_param = boost::join(loc_params,",");

    auto loc_arr = loc_obj->GetArray();

    if (loc_arr == nullptr) 
    {
        ERROR("EditModifierStringParam(0x{:016X},0x{:08X},{},{},{}) - Failed to edit the parameter",PapyrusDelegate::GetSingleton()->ToVMHandle(a_vm1,a_vm2),a_device->GetFormID(),a_modifier,a_index,a_newvalue)
        return false;
    }

    (*loc_arr)[loc_id].SetString(loc_param);

    return true;
}

std::vector<std::string> UD::ModifierManager::GetModifierAliases(uint32_t a_vm1, uint32_t a_vm2, RE::TESObjectARMO* a_device)
{
    LOG("GetModifierAliases(0x{:016X},0x{:08X}) - Called",PapyrusDelegate::GetSingleton()->ToVMHandle(a_vm1,a_vm2),a_device ? a_device->GetFormID() : 0)

    if (a_device == nullptr || ((a_vm1 + a_vm2) == 0U)) return std::vector<std::string>();

    //return all modifiers on device
    auto loc_modifiers = PapyrusDelegate::GetSingleton()->GetModifiers(PapyrusDelegate::GetSingleton()->ToVMHandle(a_vm1,a_vm2),a_device);

    std::vector<std::string> loc_res;

    for (auto&& it : loc_modifiers)
    {
        loc_res.push_back(it.namealias);
    }

    return loc_res;
}
