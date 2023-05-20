#include <UD_RegisterPapyrus.h>
#include <UD_MinigameEffect.h>

namespace UD
{
    bool RegisterPapyrusFunctions(RE::BSScript::IVirtualMachine *vm) {  
        vm->RegisterFunction("StartMinigameEffect", "UD_Native", UD::StartMinigameEffect);
        vm->RegisterFunction("EndMinigameEffect", "UD_Native", UD::EndMinigameEffect);
        vm->RegisterFunction("IsMinigameEffectOn", "UD_Native", UD::IsMinigameEffectOn);
        vm->RegisterFunction("UpdateMinigameEffectMult", "UD_Native", UD::UpdateMinigameEffectMult);
        vm->RegisterFunction("ToggleMinigameEffect", "UD_Native", UD::ToggleMinigameEffect);
        vm->RegisterFunction("MinigameStatsCheck", "UD_Native", UD::MinigameStatsCheck);
        vm->RegisterFunction("MinigameEffectUpdateHealth", "UD_Native", UD::MinigameEffectUpdateHealth);
        vm->RegisterFunction("MinigameEffectUpdateStamina", "UD_Native", UD::MinigameEffectUpdateStamina);
        vm->RegisterFunction("MinigameEffectUpdateMagicka", "UD_Native", UD::MinigameEffectUpdateMagicka);
        return true;
    }
}
