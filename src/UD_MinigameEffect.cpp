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
    void StartMinigameEffect(RE::BSScript::Internal::VirtualMachine* a_vm, const RE::VMStackID a_stackID, RE::StaticFunctionTag*, RE::Actor *a_actor, float f_mult, float f_stamina, float f_health, float f_magicka, bool b_toggle) 
    {
        //The value in papyrus was not corresponding to real value because of lag. 
        //For that reason, we need to reduce the drain value, otherwise it will be too fast
        //For now, value is reduced by 40%
        ActorValueUpdateHook::health    = -0.60f*f_health/60.0f;
        ActorValueUpdateHook::stamina   = -0.60f*f_stamina/60.0f;
        ActorValueUpdateHook::magicka   = -0.60f*f_magicka/60.0f;
        ActorValueUpdateHook::mult      = f_mult; 
        ActorValueUpdateHook::actor     = a_actor;
        ActorValueUpdateHook::toggle    = b_toggle;
        SKSE::log::info("::StartMinigameEffect - health={},stamina={},magicka={}",ActorValueUpdateHook::health,ActorValueUpdateHook::stamina,ActorValueUpdateHook::magicka);
        ActorValueUpdateHook::Patch();
    }

    void EndMinigameEffect(RE::BSScript::Internal::VirtualMachine* a_vm, const RE::VMStackID a_stackID, RE::StaticFunctionTag*, RE::Actor *a_actor) 
    {
        SKSE::log::info("::EndMinigameEffect called");
        ActorValueUpdateHook::Restore();
        ActorValueUpdateHook::health    = 0.0f;
        ActorValueUpdateHook::stamina   = 0.0f;
        ActorValueUpdateHook::magicka   = 0.0f;
        ActorValueUpdateHook::mult      = 1.0; 
        ActorValueUpdateHook::actor     = nullptr;
        ActorValueUpdateHook::toggle    = false;
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

    void MinigameEffectUpdateHealth(RE::BSScript::Internal::VirtualMachine* a_vm, const RE::VMStackID a_stackID, RE::StaticFunctionTag*, RE::Actor *a_actor, float f_health)
    {
        ActorValueUpdateHook::health    = -0.60f*f_health/60.0f;
    }
    void MinigameEffectUpdateStamina(RE::BSScript::Internal::VirtualMachine* a_vm, const RE::VMStackID a_stackID, RE::StaticFunctionTag*, RE::Actor *a_actor, float f_stamina)
    {
        ActorValueUpdateHook::stamina    = -0.60f*f_stamina/60.0f;
    }
    void MinigameEffectUpdateMagicka(RE::BSScript::Internal::VirtualMachine* a_vm, const RE::VMStackID a_stackID, RE::StaticFunctionTag*, RE::Actor *a_actor, float f_magicka)
    {
        ActorValueUpdateHook::magicka    = -0.60f*f_magicka/60.0f;
    }

	void ActorValueUpdateHook::Patch()
	{
        SKSE::log::info("ActorValueUpdateHook::Patch()");
        if (!started)
        {
			auto& trampoline = SKSE::GetTrampoline();
            trampoline.create(14);
 
			REL::Relocation<uintptr_t> ActorValueUpdateFun{ REL::RelocationID { 37509, 38451 }, 0x19 };

			ActorValueUpdate = trampoline.write_call<5>(ActorValueUpdateFun.address(), ActorValueUpdatePatched);
            started = true;
        }
	}

	void ActorValueUpdateHook::Restore()
	{
        SKSE::log::info("ActorValueUpdateHook::Restore()");
        if (ActorValueUpdate.address() != 0) 
        {
            toggle  = false;
        }
	}

	int32_t ActorValueUpdateHook::ActorValueUpdatePatched( RE::Character* a_actor, RE::ActorValue a_av, float a_unk)
	{
        //RE::ConsoleLog::GetSingleton()->Print(std::format("::ActorValueUpdate health={},stamina={},magicka={},mult={},toggle={}",health,stamina,magicka,mult,toggle).c_str());
        if ((toggle) && (a_actor != nullptr) && (actor == a_actor))
        {
            float loc_timemult = RE::GetSecondsSinceLastFrame()/(1.0f/60.0f);  //normalize to 60 fps
            if ( loc_timemult > 0.0 ) 
            {
                static RE::ActorValueOwner* loc_avholder = nullptr;
                if (loc_avholder == nullptr) loc_avholder = a_actor->AsActorValueOwner();
                _DamageAV(loc_avholder,RE::ActorValue::kHealth,health*mult*loc_timemult,5.0f);
                _DamageAV(loc_avholder,RE::ActorValue::kStamina,stamina*mult*loc_timemult,0.0);
                _DamageAV(loc_avholder,RE::ActorValue::kMagicka,magicka*mult*loc_timemult,0.0);
            }
        }
		return ActorValueUpdate(a_actor, a_av, a_unk);
	}
}