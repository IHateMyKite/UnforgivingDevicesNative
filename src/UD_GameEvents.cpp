#include <SKSE/Interfaces.h>
#include <UD_GameEvents.h>
#include <UD_Lib.h>
#include <UD_MinigameEffect.h>
#include <UD_Utility.h>
#include <UD_UI.h>
#include <UD_Inventory.h>
#include <UD_ControlManager.h>
#include <UD_PapyrusDelegate.h>
#include <UD_ModEvents.h>
#include <UD_Animation.h>
#include <UD_Skill.h>
#include <UD_ActorSlotManager.h>
#include <UD_Materials.h>
#include <UD_Diagnosis.h>
#include <UD_Lockpick.h>
#include <UD_Updater.h>
#include <UD_PlayerStatus.h>
#include <UD_Keywords.h>
#include <UD_FastTravelManager.h>
#include <UD_Modifiers.h>
#include <UD_MessageBox.h>
#include <OrgasmSystem/OrgasmManager.h>
#include <UD_DDAPI.h>
#include <UD_Macros.h>
#include <UD_ModuleManager.h>
#include <stdlib.h>
namespace UD
{
    inline void _OnGameLoad(bool a_newGame)
    {
        UD::ReloadLib();
        ModuleManager::GetSingleton()->Reload(a_newGame); // If new game, add delay, so update does not happen during char creation
        PapyrusDelegate::GetSingleton()->Setup();
        RandomGenerator::GetSingleton()->Setup();
        PlayerStatus::GetSingleton()->Setup();
        MeterManager::RemoveAll();
        KeywordManager::Reload();
        InventoryHandler::GetSingleton()->Reload();
        ORS::OrgasmManager::GetSingleton()->Setup();
        ActorSlotManager::GetSingleton()->Setup();
        ControlManager::GetSingleton()->Setup();
        MaterialManager::GetSingleton()->Setup();
        Diagnosis::GetSingleton()->Setup();
        LockpickManager::GetSingleton()->Setup();
        FastTravelManager::GetSingleton()->Setup();
        AnimationManager::GetSingleton()->Setup();
        ModifierManager::GetSingleton()->Setup();
        MessageboxManager::GetSingleton()->Setup();
        SkillManager::GetSingleton()->Setup();

        //remove effect in case that user reloaded the game without exit
        if (MinigameEffectManager::GetSingleton()->started) MinigameEffectManager::GetSingleton()->RemoveAll();

        UpdateManager::GetSingleton()->CreateUpdateThreads();
    }


    inline void _OnPostPostLoad()
    {
        //LOG("::_OnPostPostLoad called");
    }
    static std::recursive_mutex render_to_inventory_mutex;

    typedef struct {
        RE::FormID UD_DeviceKeyword_keyword;
        RE::FormID libs_quest;
        RE::FormID inventory_armor;
        RE::FormID render_armor;
        RE::FormID actor_id;
        RE::VMHandle inventory_armor_handle;
        RE::VMHandle render_armor_handle;
        RE::BSFixedString renderScript;
    } RTIData;
    static std::unordered_map<std::string, std::string> renderScriptMap;
    static std::unordered_map<std::string, std::string> inventoryScriptMap;
    static std::unordered_map<RE::FormID, RTIData> render_to_inventory_map;
    static std::unordered_map<RE::FormID, RE::BSTSmartPointer<RE::BSScript::Object>> render_to_render_object;
    static void InitScriptMaps() {
        if (renderScriptMap.size() == 0 && inventoryScriptMap.size() == 0) {
            renderScriptMap.insert(
                std::make_pair(RE::BSFixedString("zadequipscript"), RE::BSFixedString("UD_CustomDevice_RenderScript")));

            renderScriptMap.insert(
                std::make_pair(RE::BSFixedString("zadRestraintScript"), RE::BSFixedString("UD_CustomDevice_RenderScript")));

            renderScriptMap.insert(std::make_pair(RE::BSFixedString("zadRestraintFullScript"),
                                                RE::BSFixedString("UD_CustomDevice_RenderScript")));

            renderScriptMap.insert(std::make_pair(RE::BSFixedString("zadRestraintArmsScript"),
                                                RE::BSFixedString("UD_CustomDevice_RenderScript")));

            renderScriptMap.insert(std::make_pair(RE::BSFixedString("zadRestraintLegsScript"),
                                                RE::BSFixedString("UD_CustomDevice_RenderScript")));

            renderScriptMap.insert(std::make_pair(RE::BSFixedString("zadx_HobbleSkirtScript"),
                                                RE::BSFixedString("UD_CustomDevice_RenderScript")));

            renderScriptMap.insert(std::make_pair(RE::BSFixedString("zadx_RopeHarnessScript"),
                                                RE::BSFixedString("UD_CustomDevice_RenderScript")));

            renderScriptMap.insert(std::make_pair(RE::BSFixedString("zadxAnkleShacklesScript"),
                                                RE::BSFixedString("UD_CustomDevice_RenderScript")));

            renderScriptMap.insert(
                std::make_pair(RE::BSFixedString("zadGagScript"), RE::BSFixedString("UD_CustomGag_RenderScript")));

            renderScriptMap.insert(
                std::make_pair(RE::BSFixedString("zadBeltScript"), RE::BSFixedString("UD_CustomBelt_RenderScript")));

            renderScriptMap.insert(
                std::make_pair(RE::BSFixedString("zadBodyHarnessScript"), RE::BSFixedString("UD_CustomBelt_RenderScript")));

            renderScriptMap.insert(std::make_pair(RE::BSFixedString("zadBodyHarnessBeltScript"),
                                                RE::BSFixedString("UD_CustomBelt_RenderScript")));

            renderScriptMap.insert(std::make_pair(RE::BSFixedString("zadBlockingHarnessScript"),
                                                RE::BSFixedString("UD_CustomBelt_RenderScript")));

            renderScriptMap.insert(std::make_pair(RE::BSFixedString("zadxChainHarnessNippleChainScript"),
                                                RE::BSFixedString("UD_CustomBelt_RenderScript")));

            renderScriptMap.insert(std::make_pair(RE::BSFixedString("zadBodyNoCollarHarnessScript"),
                                                RE::BSFixedString("UD_CustomBelt_RenderScript")));

            renderScriptMap.insert(std::make_pair(RE::BSFixedString("zadBodyNoCollarHarnessBeltScript"),
                                                RE::BSFixedString("UD_CustomBelt_RenderScript")));

            renderScriptMap.insert(
                std::make_pair(RE::BSFixedString("zadCorsetBeltScript"), RE::BSFixedString("UD_CustomBelt_RenderScript")));

            renderScriptMap.insert(
                std::make_pair(RE::BSFixedString("zadBraScript"), RE::BSFixedString("UD_CustomBra_RenderScript")));

            renderScriptMap.insert(
                std::make_pair(RE::BSFixedString("zadCorsetScript"), RE::BSFixedString("UD_CustomCorset_RenderScript")));

            renderScriptMap.insert(
                std::make_pair(RE::BSFixedString("zadPlugScript"), RE::BSFixedString("UD_CustomPlug_RenderScript")));

            renderScriptMap.insert(
                std::make_pair(RE::BSFixedString("zadPlugSoulgemScript"), RE::BSFixedString("UD_CustomPlug_RenderScript")));

            renderScriptMap.insert(std::make_pair(RE::BSFixedString("zadPlugPumpsScript"),
                                                RE::BSFixedString("UD_CustomInflatablePlug_RenderScript")));

            renderScriptMap.insert(std::make_pair(RE::BSFixedString("zadRestraintArmBinderScript"),
                                                RE::BSFixedString("UD_CustomHeavyBondage_RenderScript")));

            renderScriptMap.insert(std::make_pair(RE::BSFixedString("zadBlockingBlindfoldScript"),
                                                RE::BSFixedString("UD_CustomBlindfold_RenderScript")));

            renderScriptMap.insert(std::make_pair(RE::BSFixedString("zadBlindfoldScript"),
                                                RE::BSFixedString("UD_CustomBlindfold_RenderScript")));

            renderScriptMap.insert(
                std::make_pair(RE::BSFixedString("zadSlaveBootsScript"), RE::BSFixedString("UD_CustomBoots_RenderScript")));

            renderScriptMap.insert(std::make_pair(RE::BSFixedString("zadSlaveBootsUnlockedScript"),
                                                RE::BSFixedString("UD_CustomBoots_RenderScript")));

            renderScriptMap.insert(
                std::make_pair(RE::BSFixedString("zadCollarScript"), RE::BSFixedString("UD_CustomCollar_RenderScript")));

            renderScriptMap.insert(std::make_pair(RE::BSFixedString("zadCollarNPiercingsScript"),
                                                RE::BSFixedString("UD_CustomCollar_RenderScript")));

            renderScriptMap.insert(
                std::make_pair(RE::BSFixedString("zadGlovesScript"), RE::BSFixedString("UD_CustomGloves_RenderScript")));

            renderScriptMap.insert(std::make_pair(RE::BSFixedString("zadPiercingVaginalScript"),
                                                RE::BSFixedString("UD_CustomPiercing_RenderScript")));

            renderScriptMap.insert(std::make_pair(RE::BSFixedString("zadPiercingNippleScript"),
                                                RE::BSFixedString("UD_CustomPiercing_RenderScript")));

            renderScriptMap.insert(std::make_pair(RE::BSFixedString("zadPlugChargeableScript"),
                                                RE::BSFixedString("UD_CustomChargPlug_RenderScript")));

            renderScriptMap.insert(std::make_pair(RE::BSFixedString("zadGagPanelScript"),
                                                RE::BSFixedString("UD_CustomPanelGag_RenderScript")));

            renderScriptMap.insert(std::make_pair(RE::BSFixedString("zadx_BondageMittensScript"),
                                                RE::BSFixedString("UD_CustomMittens_RenderScript")));

            inventoryScriptMap.insert(
                std::make_pair(RE::BSFixedString("zadequipscript"), RE::BSFixedString("UD_CustomDevice_EquipScript")));

            inventoryScriptMap.insert(
                std::make_pair(RE::BSFixedString("zadGagScript"), RE::BSFixedString("UD_CustomDevice_EquipScript")));

            inventoryScriptMap.insert(
                std::make_pair(RE::BSFixedString("zadBeltScript"), RE::BSFixedString("UD_CustomDevice_EquipScript")));

            inventoryScriptMap.insert(
                std::make_pair(RE::BSFixedString("zadCorsetScript"), RE::BSFixedString("UD_CustomDevice_EquipScript")));

            inventoryScriptMap.insert(
                std::make_pair(RE::BSFixedString("zadPlugScript"), RE::BSFixedString("UD_CustomDevice_EquipScript")));

            inventoryScriptMap.insert(std::make_pair(RE::BSFixedString("zadPlugSoulgemScript"),
                                                    RE::BSFixedString("UD_CustomDevice_EquipScript")));

            inventoryScriptMap.insert(std::make_pair(RE::BSFixedString("zadPlugChargeableScript"),
                                                    RE::BSFixedString("UD_CustomDevice_EquipScript")));

            inventoryScriptMap.insert(
                std::make_pair(RE::BSFixedString("zadPlugPumpsScript"), RE::BSFixedString("UD_CustomDevice_EquipScript")));

            inventoryScriptMap.insert(
                std::make_pair(RE::BSFixedString("zadGagPanelScript"), RE::BSFixedString("UD_CustomDevice_EquipScript")));

            inventoryScriptMap.insert(std::make_pair(RE::BSFixedString("zadBodyHarnessScript"),
                                                    RE::BSFixedString("UD_CustomDevice_EquipScript")));

            inventoryScriptMap.insert(std::make_pair(RE::BSFixedString("zadBodyHarnessBeltScript"),
                                                    RE::BSFixedString("UD_CustomDevice_EquipScript")));

            inventoryScriptMap.insert(std::make_pair(RE::BSFixedString("zadBlockingHarnessScript"),
                                                    RE::BSFixedString("UD_CustomDevice_EquipScript")));

            inventoryScriptMap.insert(
                std::make_pair(RE::BSFixedString("zadRestraintScript"), RE::BSFixedString("UD_CustomDevice_EquipScript")));

            inventoryScriptMap.insert(std::make_pair(RE::BSFixedString("zadRestraintFullScript"),
                                                    RE::BSFixedString("UD_CustomDevice_EquipScript")));

            inventoryScriptMap.insert(std::make_pair(RE::BSFixedString("zadRestraintArmsScript"),
                                                    RE::BSFixedString("UD_CustomDevice_EquipScript")));

            inventoryScriptMap.insert(std::make_pair(RE::BSFixedString("zadRestraintLegsScript"),
                                                    RE::BSFixedString("UD_CustomDevice_EquipScript")));

            inventoryScriptMap.insert(std::make_pair(RE::BSFixedString("zadRestraintArmBinderScript"),
                                                    RE::BSFixedString("UD_CustomDevice_EquipScript")));

            inventoryScriptMap.insert(std::make_pair(RE::BSFixedString("zadBlockingBlindfoldScript"),
                                                    RE::BSFixedString("UD_CustomDevice_EquipScript")));

            inventoryScriptMap.insert(
                std::make_pair(RE::BSFixedString("zadBlindfoldScript"), RE::BSFixedString("UD_CustomDevice_EquipScript")));

            inventoryScriptMap.insert(
                std::make_pair(RE::BSFixedString("zadSlaveBootsScript"), RE::BSFixedString("UD_CustomDevice_EquipScript")));

            inventoryScriptMap.insert(
                std::make_pair(RE::BSFixedString("zadBlindfoldScript"), RE::BSFixedString("UD_CustomDevice_EquipScript")));

            inventoryScriptMap.insert(std::make_pair(RE::BSFixedString("zadSlaveBootsUnlockedScript"),
                                                    RE::BSFixedString("UD_CustomDevice_EquipScript")));

            inventoryScriptMap.insert(
                std::make_pair(RE::BSFixedString("zadCollarScript"), RE::BSFixedString("UD_CustomDevice_EquipScript")));

            inventoryScriptMap.insert(std::make_pair(RE::BSFixedString("zadxChainHarnessNippleChainScript"),
                                                    RE::BSFixedString("UD_CustomDevice_EquipScript")));

            inventoryScriptMap.insert(
                std::make_pair(RE::BSFixedString("zadGlovesScript"), RE::BSFixedString("UD_CustomDevice_EquipScript")));

            inventoryScriptMap.insert(std::make_pair(RE::BSFixedString("zadCollarNPiercingsScript"),
                                                    RE::BSFixedString("UD_CustomDevice_EquipScript")));

            inventoryScriptMap.insert(
                std::make_pair(RE::BSFixedString("zadBraScript"), RE::BSFixedString("UD_CustomDevice_EquipScript")));

            inventoryScriptMap.insert(std::make_pair(RE::BSFixedString("zadx_HobbleSkirtScript"),
                                                    RE::BSFixedString("UD_CustomDevice_EquipScript")));

            inventoryScriptMap.insert(std::make_pair(RE::BSFixedString("zadx_RopeHarnessScript"),
                                                    RE::BSFixedString("UD_CustomDevice_EquipScript")));

            inventoryScriptMap.insert(std::make_pair(RE::BSFixedString("zadx_BondageMittensScript"),
                                                    RE::BSFixedString("UD_CustomDevice_EquipScript")));

            inventoryScriptMap.insert(std::make_pair(RE::BSFixedString("zadxAnkleShacklesScript"),
                                                    RE::BSFixedString("UD_CustomDevice_EquipScript")));

            inventoryScriptMap.insert(std::make_pair(RE::BSFixedString("zadPiercingVaginalScript"),
                                                    RE::BSFixedString("UD_CustomDevice_EquipScript")));

            inventoryScriptMap.insert(std::make_pair(RE::BSFixedString("zadPiercingNippleScript"),
                                                    RE::BSFixedString("UD_CustomDevice_EquipScript")));

            inventoryScriptMap.insert(std::make_pair(RE::BSFixedString("zadBodyNoCollarHarnessScript"),
                                                    RE::BSFixedString("UD_CustomDevice_EquipScript")));

            inventoryScriptMap.insert(std::make_pair(RE::BSFixedString("zadBodyNoCollarHarnessBeltScript"),
                                                    RE::BSFixedString("UD_CustomDevice_EquipScript")));

            inventoryScriptMap.insert(
                std::make_pair(RE::BSFixedString("zadCorsetBeltScript"), RE::BSFixedString("UD_CustomDevice_EquipScript")));
        }
    }

    typedef struct RawProp {
        RE::BSFixedString name;
        uint64_t prop_type;
        uint64_t raw_value;
        uint64_t array_value;
    } RawProp;
    typedef struct RawScript {
        uint64_t unk_one;                  // 00
        RE::BSFixedString script_name;     // 08
        uint64_t prop_dword_count;  // 10
        RawProp props[]; // 18
    } RawScript;
    typedef struct RenderPatchInfo {
        RawScript *render_raw_script;
        RE::VMHandle render_handle;
    } RenderPatchInfo;

    RenderPatchInfo PatchInvScript(RawScript &script) 
    {
        auto vm = RE::BSScript::Internal::VirtualMachine::GetSingleton();
        if (!vm) {
            return RenderPatchInfo{nullptr, 0x0000FFFF00000000};
        }
        auto policy = vm->GetObjectHandlePolicy();
        if (!policy) {
            return RenderPatchInfo{nullptr, 0x0000FFFF00000000};
        }
        auto dataHandler = RE::TESDataHandler::GetSingleton();
        if (!dataHandler) {
            return RenderPatchInfo{nullptr, 0x0000FFFF00000000};
        }
        InitScriptMaps();
        auto zadinvkw = dataHandler->LookupForm<RE::BGSKeyword>(0x02b5f0, "Devious Devices - Integration.esm");
        auto zadlibs = dataHandler->LookupForm<RE::TESQuest>(0xf624, "Devious Devices - Integration.esm");
        auto udinvkw = dataHandler->LookupForm<RE::BGSKeyword>(0x1553dd, "UnforgivingDevices.esp");
        auto udpatchkw = dataHandler->LookupForm<RE::BGSKeyword>(0x13a977, "UnforgivingDevices.esp");
        auto udkw = dataHandler->LookupForm<RE::BGSKeyword>(0x11a352, "UnforgivingDevices.esp");
        auto udpatchnmkw = dataHandler->LookupForm<RE::BGSKeyword>(0x1579be, "UnforgivingDevices.esp");
        auto udmq = dataHandler->LookupForm<RE::TESQuest>(0x15e73c, "UnforgivingDevices.esp");
        if (!zadinvkw || !udinvkw || !udpatchkw || !udkw || !udpatchnmkw || !udmq || !zadlibs) {
            ERROR("Failed to get all keywords for patcher");
            return RenderPatchInfo{nullptr, 0x0000FFFF00000000};
        }
        
        if (inventoryScriptMap.contains(std::string(script.script_name))) {

            uint32_t prop_count = ((script.prop_dword_count&0xFFFFFFFF) / 8);
            auto new_inv_script_name = RE::BSFixedString(inventoryScriptMap[std::string(script.script_name)]);
            auto new_render_script_name = RE::BSFixedString(renderScriptMap[std::string(script.script_name)]);
            script.script_name = new_inv_script_name;
            RE::VMHandle render_handle = 0x0000FFFF00000000;
            RE::VMHandle inv_handle = 0x0000FFFF00000000;
            RE::VMHandle libs_handle = 0x0000FFFF00000000;
            RE::VMHandle dkw_handle = 0x0000FFFF00000000;
            auto& props = script.props;
            for (uint32_t p_i = 0; p_i < prop_count; p_i += 1) {
                if (props[p_i].name == RE::BSFixedString("deviceRendered")) {
                    render_handle = props[p_i].raw_value;
                    
                }
                if (props[p_i].name == RE::BSFixedString("deviceInventory")) {
                    inv_handle = props[p_i].raw_value;
                }
                if (auto render_armor = RE::TESForm::LookupByID<RE::TESObjectARMO>(render_handle & 0xFFFFFFFF)) {
                    if (auto inventory_armor = RE::TESForm::LookupByID<RE::TESObjectARMO>(inv_handle & 0xFFFFFFFF)) {
                        if (!inventory_armor->HasKeyword(udinvkw)) {
                            inventory_armor->AddKeyword(udinvkw);
                        }

                        if (!render_armor->HasKeyword(udkw)) {
                            render_armor->AddKeyword(udkw);
                            render_armor->AddKeyword(udpatchkw);
                        }
                    }
                }
                if (props[p_i].name == RE::BSFixedString("libs")) {
                    libs_handle = props[p_i].raw_value;
                }
                if (props[p_i].name == RE::BSFixedString("zad_DeviousDevice")) {
                    dkw_handle = props[p_i].raw_value;
                }
            }
            auto render_prop_count = 3;
            RawScript *new_render_script = (RawScript*)_aligned_malloc(0x18 + (0x20 * render_prop_count)+0x20,0x20);
            memset(new_render_script, 0x0, 0x18 + (0x20 * render_prop_count)+0x20);
            new_render_script->unk_one = 1;
            new_render_script->script_name = new_render_script_name;
            new_render_script->prop_dword_count = 8 * render_prop_count;
            auto& render_props = new_render_script->props;
            render_props[0].name = RE::BSFixedString("UD_DeviceKeyword");
            render_props[0].prop_type = 0x10001;
            render_props[0].raw_value = (uint64_t)dkw_handle;
            render_props[0].array_value = 0x0;
            render_props[1].name = RE::BSFixedString("libs");
            render_props[1].prop_type = 0x10001;
            render_props[1].raw_value = (uint64_t)libs_handle;
            render_props[1].array_value = 0x0;
            render_props[2].name = RE::BSFixedString("deviceInventory");
            render_props[2].prop_type = 0x10001;
            render_props[2].raw_value = (uint64_t)inv_handle;
            render_props[2].array_value = 0x0;
            //memset(&render_props[3], 0x0,0x20);
            return RenderPatchInfo{new_render_script, render_handle};
            //RE::BSScript::Internal::VirtualMachine::GetSingleton()->GetScriptObjectTypeNoLoad(new_script_name,script_info);
            
        }
        return RenderPatchInfo{nullptr, 0x0000FFFF00000000};
    }
    void PatcherNewGame(bool skip_scripts) 
    {
        auto vm = RE::BSScript::Internal::VirtualMachine::GetSingleton();
        if (!vm) {
            return;
        }
        auto policy = vm->GetObjectHandlePolicy();
        if (!policy) {
            return;
        }
        auto dataHandler = RE::TESDataHandler::GetSingleton();
        if (!dataHandler) {
            return;
        }
        InitScriptMaps();
        auto zadinvkw = dataHandler->LookupForm<RE::BGSKeyword>(0x02b5f0, "Devious Devices - Integration.esm");
        auto zadlibs = dataHandler->LookupForm<RE::TESQuest>(0xf624, "Devious Devices - Integration.esm");
        auto zadquestitem = dataHandler->LookupForm<RE::BGSKeyword>(0x5aea8, "Devious Devices - Integration.esm");
        auto udinvkw = dataHandler->LookupForm<RE::BGSKeyword>(0x1553dd, "UnforgivingDevices.esp");
        auto udpatchkw = dataHandler->LookupForm<RE::BGSKeyword>(0x13a977, "UnforgivingDevices.esp");
        auto udkw = dataHandler->LookupForm<RE::BGSKeyword>(0x11a352, "UnforgivingDevices.esp");
        auto udpatchnmkw = dataHandler->LookupForm<RE::BGSKeyword>(0x1579be, "UnforgivingDevices.esp");
        auto udmq = dataHandler->LookupForm<RE::TESQuest>(0x15e73c, "UnforgivingDevices.esp");
        if (!zadinvkw || !udinvkw || !udpatchkw || !udkw || !udpatchnmkw || !udmq || !zadlibs || !zadquestitem) {
            ERROR("Failed to get all keywords for patcher");
            return;
        }
        
        auto armor_array = dataHandler->GetFormArray<RE::TESObjectARMO>();
        auto zad_inv_armor_ids = std::unordered_set<RE::FormID>();
        for (auto ar : armor_array) {
            if (!ar) {
                continue;
            }
            if (ar->HasKeyword(zadquestitem)) {
                continue;
            }
            if (ar->HasKeyword(zadinvkw)) {
                zad_inv_armor_ids.insert(ar->formID);
            }
            for (auto &kw : ar->GetKeywords()) {
                if (!kw) {
                    continue;
                }
                if (!kw->GetFile()) {
                    continue;
                }
                if (RE::BSFixedString(kw->GetFile()->GetFilename()) ==
                    RE::BSFixedString("Devious Devices - Integration.esm")) {
                    ar->formFlags |= RE::TESForm::RecordFlags::kFormRetainsID;
                    //ar->InitItemImpl();
                }
            }
        }
        if (skip_scripts) {
            return;
        }
        auto &attached_scripts = *(RE::BSTHashMap<RE::VMHandle, RE::BSTSmallSharedArray<RawScript *>> *)( &
                                (RE::SkyrimVM::GetSingleton()
                ->GetImpl()
                ->GetObjectBindPolicy()
                ->attachedScripts));
        RE::SkyrimVM::GetSingleton()->GetImpl()->GetObjectBindPolicy()->attachedScriptsLock.Lock();
        std::vector <RenderPatchInfo> patch_results;
        for (auto &p : attached_scripts) {

            if (!zad_inv_armor_ids.contains(RE::FormID(p.first & 0xFFFFFFFF))) {
                continue;
            }
            
            auto scripts=((RE::BSTSmallSharedArray<RawScript*>*)&p.second);
            ERROR("Patcher New Game Handle {:016X} Script Count {}", p.first, scripts->size());
            for (auto &s : *scripts) {
                //if (s->unk_one==1) {
                if ((s->unk_one&0xFFFFFFFF) != 1) {
                    ERROR("Patcher New Game Handle {:016X} Script Name {} unk_one {:08X}", p.first, s->script_name, s->unk_one&0xFFFFFFFF);
                }
                ERROR("Patcher New Game Handle {:016X} Script Name {}", p.first, s->script_name);
                patch_results.push_back(PatchInvScript(*s));
                    
                //}
            }
        }
        for (auto &r : patch_results) {
            if (r.render_raw_script && r.render_handle != 0x0000FFFF00000000) {
                auto script_array =
                    (RE::BSTSmallSharedArray<RawScript *> *)_aligned_malloc(0x20,0x20);
                memset(script_array, 0, 0x20);
                (*script_array)[0] = r.render_raw_script;
                *(unsigned int *)script_array = 0x1;
                if (attached_scripts.contains(r.render_handle)) {
                    (attached_scripts.find(r.render_handle))->second = *script_array;
                } else {
                    attached_scripts.insert(RE::BSTTuple(r.render_handle, *script_array));
                    RE::SkyrimVM::GetSingleton()->GetImpl()->GetObjectBindPolicy()->Unk_03(r.render_handle);
                }

            }
         
        }
        RE::SkyrimVM::GetSingleton()->GetImpl()->GetObjectBindPolicy()->attachedScriptsLock.Unlock();
    }
 
    void OnMessageReceived(SKSE::MessagingInterface::Message* a_msg)
    {
        Config::GetSingleton()->Setup();
        bool nativeDDNGPatcher=Config::GetSingleton()->GetVariable<bool>("General.bNativeDDNGPatcher",true);
        switch(a_msg->type)
        {
            case SKSE::MessagingInterface::kInputLoaded:
                RE::BSInputDeviceManager::GetSingleton()->AddEventSink(KeyEventSink::GetSingleton());
                break;
            case SKSE::MessagingInterface::kDataLoaded:
                
                if (nativeDDNGPatcher) {
                    InitScriptMaps();
                    PatcherNewGame(false);
                }
                break;
            case SKSE::MessagingInterface::kPostLoadGame:
                if (!DeviousDevicesAPI::LoadAPI()) ERROR("Could not load DD API!")
                // PatcherNewGame();
                if (nativeDDNGPatcher) {
                    PatcherNewGame(true);
                }
                _OnGameLoad(false);
                break;
            case SKSE::MessagingInterface::kNewGame:
                if (!DeviousDevicesAPI::LoadAPI()) ERROR("Could not load DD API!")
                if (nativeDDNGPatcher) {
                    PatcherNewGame(true);
                }
                _OnGameLoad(true);
                break;
            case SKSE::MessagingInterface::kPostPostLoad:
                _OnPostPostLoad();
                break;
        }
    }
}