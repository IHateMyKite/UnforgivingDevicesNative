#include <UD_RegisterPapyrus.h>
#include <UD_MinigameEffect.h>
#include <UD_Utility.h>
#include <UD_UI.h>
#include <UD_Inventory.h>

namespace UD
{
    #define REGISTERPAPYRUSFUNC(name) vm->RegisterFunction(#name, "UD_Native", UD::name);

    bool RegisterPapyrusFunctions(RE::BSScript::IVirtualMachine *vm) {
        REGISTERPAPYRUSFUNC(StartMinigameEffect)
        REGISTERPAPYRUSFUNC(EndMinigameEffect)
        REGISTERPAPYRUSFUNC(IsMinigameEffectOn)
        REGISTERPAPYRUSFUNC(UpdateMinigameEffectMult)
        REGISTERPAPYRUSFUNC(ToggleMinigameEffect)
        REGISTERPAPYRUSFUNC(MinigameStatsCheck)
        REGISTERPAPYRUSFUNC(MinigameEffectUpdateHealth)
        REGISTERPAPYRUSFUNC(MinigameEffectUpdateStamina)
        REGISTERPAPYRUSFUNC(MinigameEffectUpdateMagicka)

        //UTILITY
        REGISTERPAPYRUSFUNC(CodeBit)
        REGISTERPAPYRUSFUNC(DecodeBit)

        //UI
        REGISTERPAPYRUSFUNC(AddMeterEntryIWW)
        REGISTERPAPYRUSFUNC(RemoveMeterEntryIWW)
        REGISTERPAPYRUSFUNC(ToggleMeterIWW)
        REGISTERPAPYRUSFUNC(SetMeterRateIWW)
        REGISTERPAPYRUSFUNC(SetMeterMultIWW)
        REGISTERPAPYRUSFUNC(SetMeterValueIWW)
        REGISTERPAPYRUSFUNC(UpdateMeterValueIWW)
        REGISTERPAPYRUSFUNC(GetMeterValueIWW)

        REGISTERPAPYRUSFUNC(AddMeterEntrySkyUi)
        REGISTERPAPYRUSFUNC(RemoveMeterEntrySkyUi)
        REGISTERPAPYRUSFUNC(ToggleMeterSkyUi)
        REGISTERPAPYRUSFUNC(SetMeterRateSkyUi)
        REGISTERPAPYRUSFUNC(SetMeterMultSkyUi)
        REGISTERPAPYRUSFUNC(SetMeterValueSkyUi)
        REGISTERPAPYRUSFUNC(UpdateMeterValueSkyUi)
        REGISTERPAPYRUSFUNC(GetMeterValueSkyUi)

        REGISTERPAPYRUSFUNC(ToggleAllMeters)
        REGISTERPAPYRUSFUNC(RemoveAllMeterEntries)


        //Inventory
        REGISTERPAPYRUSFUNC(GetInventoryDevices)
        REGISTERPAPYRUSFUNC(GetRenderDevices)
        REGISTERPAPYRUSFUNC(GetSharpestWeapon)
        return true;
    }

    #undef REGISTERPAPYRUSFUNC
}
