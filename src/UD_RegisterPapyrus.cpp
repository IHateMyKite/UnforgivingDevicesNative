#include <UD_RegisterPapyrus.h>
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
#include <OrgasmSystem/OrgasmManager.h>

namespace UD
{
    #define REGISTERPAPYRUSFUNC(name,unhook) vm->RegisterFunction(#name, "UD_Native", UD::name,unhook);

    bool RegisterPapyrusFunctions(RE::BSScript::IVirtualMachine *vm) {
        REGISTERPAPYRUSFUNC(StartMinigameEffect,true)
        REGISTERPAPYRUSFUNC(EndMinigameEffect,true)
        REGISTERPAPYRUSFUNC(IsMinigameEffectOn,true)
        REGISTERPAPYRUSFUNC(UpdateMinigameEffectMult,true)
        REGISTERPAPYRUSFUNC(ToggleMinigameEffect,true)
        REGISTERPAPYRUSFUNC(MinigameStatsCheck,true)
        REGISTERPAPYRUSFUNC(MinigameEffectSetHealth,true)
        REGISTERPAPYRUSFUNC(MinigameEffectSetStamina,true)
        REGISTERPAPYRUSFUNC(MinigameEffectSetMagicka,true)
        REGISTERPAPYRUSFUNC(MinigameEffectUpdateHealth,true)
        REGISTERPAPYRUSFUNC(MinigameEffectUpdateStamina,true)
        REGISTERPAPYRUSFUNC(MinigameEffectUpdateMagicka,true)

        //UTILITY
        REGISTERPAPYRUSFUNC(CodeBit,true)
        REGISTERPAPYRUSFUNC(DecodeBit,true)
        REGISTERPAPYRUSFUNC(iRange,true)
        REGISTERPAPYRUSFUNC(fRange,true)
        REGISTERPAPYRUSFUNC(iInRange,true)
        REGISTERPAPYRUSFUNC(fInRange,true)
        REGISTERPAPYRUSFUNC(Round,true)
        REGISTERPAPYRUSFUNC(FormatFloat,true)
        REGISTERPAPYRUSFUNC(IsPlayer,true)
        REGISTERPAPYRUSFUNC(GetActorName,true)
        REGISTERPAPYRUSFUNC(FloatToInt,true)
        REGISTERPAPYRUSFUNC(RandomFloat,true)
        REGISTERPAPYRUSFUNC(RandomInt,true)
        REGISTERPAPYRUSFUNC(RandomIdFromDist,true)
        REGISTERPAPYRUSFUNC(PluginInstalled,true)
        REGISTERPAPYRUSFUNC(CheckArmorEquipped,true)
        REGISTERPAPYRUSFUNC(ToggleDetection,true)
        REGISTERPAPYRUSFUNC(GetStringParamAll,true)
        REGISTERPAPYRUSFUNC(GetStringParamInt,true)
        REGISTERPAPYRUSFUNC(GetStringParamFloat,true)
        REGISTERPAPYRUSFUNC(GetStringParamString,true)
        REGISTERPAPYRUSFUNC(GetRandomDevice,true)

        //UI
        REGISTERPAPYRUSFUNC(AddMeterEntryIWW,true)
        REGISTERPAPYRUSFUNC(RemoveMeterEntryIWW,true)
        REGISTERPAPYRUSFUNC(ToggleMeterIWW,true)
        REGISTERPAPYRUSFUNC(SetMeterRateIWW,true)
        REGISTERPAPYRUSFUNC(SetMeterMultIWW,true)
        REGISTERPAPYRUSFUNC(SetMeterValueIWW,true)
        REGISTERPAPYRUSFUNC(UpdateMeterValueIWW,true)
        REGISTERPAPYRUSFUNC(GetMeterValueIWW,true)

        REGISTERPAPYRUSFUNC(AddMeterEntrySkyUi,true)
        REGISTERPAPYRUSFUNC(RemoveMeterEntrySkyUi,true)
        REGISTERPAPYRUSFUNC(ToggleMeterSkyUi,true)
        REGISTERPAPYRUSFUNC(SetMeterRateSkyUi,true)
        REGISTERPAPYRUSFUNC(SetMeterMultSkyUi,true)
        REGISTERPAPYRUSFUNC(SetMeterValueSkyUi,true)
        REGISTERPAPYRUSFUNC(UpdateMeterValueSkyUi,true)
        REGISTERPAPYRUSFUNC(GetMeterValueSkyUi,true)

        REGISTERPAPYRUSFUNC(ToggleAllMeters,true)
        REGISTERPAPYRUSFUNC(RemoveAllMeterEntries,true)

        //Inventory
        REGISTERPAPYRUSFUNC(GetInventoryDevices,true)
        REGISTERPAPYRUSFUNC(GetRenderDevices,true)
        REGISTERPAPYRUSFUNC(GetSharpestWeapon,true)

        //animation
        REGISTERPAPYRUSFUNC(GetActorConstrains,true)
        REGISTERPAPYRUSFUNC(CheckWeaponDisabled,true)
        REGISTERPAPYRUSFUNC(DisableWeapons,true)

        //skill
        REGISTERPAPYRUSFUNC(CalculateSkillFromPerks,true)

        //actor slot manager
        REGISTERPAPYRUSFUNC(RegisterSlotQuest,true)
        REGISTERPAPYRUSFUNC(GetRegisteredActors,true)

        //player control
        REGISTERPAPYRUSFUNC(SyncControlSetting,true)
        REGISTERPAPYRUSFUNC(GetCameraState,true)
        REGISTERPAPYRUSFUNC(RegisterDeviceCallback,true)
        REGISTERPAPYRUSFUNC(UnregisterDeviceCallbacks,true)
        REGISTERPAPYRUSFUNC(UnregisterAllDeviceCallbacks,true)
        REGISTERPAPYRUSFUNC(AddDeviceCallbackArgument,true)

        //events
        REGISTERPAPYRUSFUNC(RegisterForHMTweenMenu,true)

        //object handler
        REGISTERPAPYRUSFUNC(SendRegisterDeviceScriptEvent,true)
        REGISTERPAPYRUSFUNC(SendMinigameThreadEvents,true)
        REGISTERPAPYRUSFUNC(SendRemoveRenderDeviceEvent,true)
        REGISTERPAPYRUSFUNC(SetBitMapData,true)
        REGISTERPAPYRUSFUNC(UpdateVMHandles,true)

        //materials
        REGISTERPAPYRUSFUNC(IsSteel,true)
        REGISTERPAPYRUSFUNC(IsEbonite,true)
        REGISTERPAPYRUSFUNC(IsRope,true)
        REGISTERPAPYRUSFUNC(IsSecure,true)
        REGISTERPAPYRUSFUNC(IsLeather,true)

        //diagnosis
        REGISTERPAPYRUSFUNC(CheckPatchedDevices,true)

        //lockpick
        REGISTERPAPYRUSFUNC(GetLockpickVariable,true)
        REGISTERPAPYRUSFUNC(SetLockpickVariable,true)

        ORS::OrgasmManager::GetSingleton()->RegisterPapyrusFunctions(vm);

        return true;
    }

    #undef REGISTERPAPYRUSFUNC
}
