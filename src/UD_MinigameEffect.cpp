#include <UD_MinigameEffect.h>
namespace UD
{
    std::atomic<float> ActorValueUpdateHook::health     = std::atomic<float>(0.0f);
    std::atomic<float> ActorValueUpdateHook::stamina    = std::atomic<float>(0.0f);
    std::atomic<float> ActorValueUpdateHook::magicka    = std::atomic<float>(0.0f);
    std::atomic<float> ActorValueUpdateHook::mult       = std::atomic<float>(1.0f);
    RE::Actor* ActorValueUpdateHook::actor              = nullptr;
    std::atomic<bool> ActorValueUpdateHook::started     = std::atomic<bool>(false);
    std::atomic<bool> ActorValueUpdateHook::toggle      = std::atomic<bool>(true);


    bool _DamageAV(RE::ActorValueOwner* a_avowner,const RE::ActorValue& a_av, const float& f_dmg, const float& f_min)
    {
        if ((f_dmg != 0.0) && ((a_avowner->GetActorValue(a_av) - f_dmg) > f_min))
        {
            a_avowner->RestoreActorValue(RE::ACTOR_VALUE_MODIFIER::kDamage,a_av,f_dmg);
        }
        return true;
    }

    // Function ProcessAVMinigame(Actor akActor, Float afStamina, Float afHealth, Float afMagicka) global native
    void StartMinigameEffect(RE::BSScript::Internal::VirtualMachine* a_vm, const RE::VMStackID a_stackID, RE::StaticFunctionTag*, RE::Actor *a_actor, float f_mult, float f_stamina, float f_health, float f_magicka) 
    {
        //The value in papyrus was not corresponding to real value because of lag. 
        //For that reason, we need to reduce the drain value, otherwise it will be too fast
        //For now, value is reduced by 25%
        ActorValueUpdateHook::health    = -0.75f*f_health/60.0f;
        ActorValueUpdateHook::stamina   = -0.75f*f_stamina/60.0f;
        ActorValueUpdateHook::magicka   = -0.75f*f_magicka/60.0f;
        ActorValueUpdateHook::mult      = f_mult; 
        ActorValueUpdateHook::actor     = a_actor;
        SKSE::log::info("::StartMinigameEffect - health={},stamina={},magicka={}",ActorValueUpdateHook::health,ActorValueUpdateHook::stamina,ActorValueUpdateHook::magicka);
        ActorValueUpdateHook::Patch();
    }

    void EndMinigameEffect(RE::BSScript::Internal::VirtualMachine* a_vm, const RE::VMStackID a_stackID, RE::StaticFunctionTag*, RE::Actor *a_actor) 
    {
        SKSE::log::info("::EndMinigameEffect called");
        ActorValueUpdateHook::Restore();
    }

    bool IsMinigameEffectOn(RE::BSScript::Internal::VirtualMachine* a_vm, const RE::VMStackID a_stackID, RE::StaticFunctionTag*, RE::Actor *a_actor) 
    {
        return ActorValueUpdateHook::started;
    }

    void UpdateMinigameEffectMult(RE::BSScript::Internal::VirtualMachine* a_vm, const RE::VMStackID a_stackID, RE::StaticFunctionTag*, RE::Actor *a_actor, float f_newvalue) 
    {
        ActorValueUpdateHook::mult = f_newvalue;
    }

    void ToggleMinigameEffect(RE::BSScript::Internal::VirtualMachine* a_vm, const RE::VMStackID a_stackID, RE::StaticFunctionTag*, RE::Actor *a_actor, bool b_toggle) 
    {
        SKSE::log::info("::ToggleMinigameEffect called {}",b_toggle);
        RE::ConsoleLog::GetSingleton()->Print("ActorValueUpdateHook::ToggleMinigameEffect()");
        ActorValueUpdateHook::toggle = b_toggle;
    }

    bool MinigameStatsCheck(RE::BSScript::Internal::VirtualMachine* a_vm, const RE::VMStackID a_stackID, RE::StaticFunctionTag*, RE::Actor *a_actor) 
    {
        RE::ActorValueOwner* a_avowner   = a_actor->AsActorValueOwner();
        bool loc_res = true;
        loc_res &= a_avowner->GetActorValue(RE::ActorValue::kStamina) > 0.1;
        loc_res &= a_avowner->GetActorValue(RE::ActorValue::kHealth)  > 5.1;
        loc_res &= a_avowner->GetActorValue(RE::ActorValue::kMagicka) > 0.1;
        return loc_res;
    }
}