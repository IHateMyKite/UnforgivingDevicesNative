#include <UD_RegisterPapyrus.h>
#include <UD_MinigameEffect.h>
#include <UD_Utility.h>
#include <UD_UI.h>

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
        REGISTERPAPYRUSFUNC(AddMeterEntry)
        REGISTERPAPYRUSFUNC(RemoveMeterEntry)
        REGISTERPAPYRUSFUNC(ToggleAllMeters)
        REGISTERPAPYRUSFUNC(ToggleMeter)
        REGISTERPAPYRUSFUNC(SetMeterRate)
        REGISTERPAPYRUSFUNC(SetMeterMult)
        REGISTERPAPYRUSFUNC(SetMeterValue)
        REGISTERPAPYRUSFUNC(UpdateMeterValue)
        REGISTERPAPYRUSFUNC(GetMeterValue)
        REGISTERPAPYRUSFUNC(RemoveAllMeterEntries)
        return true;
    }

    #undef REGISTERPAPYRUSFUNC
}
