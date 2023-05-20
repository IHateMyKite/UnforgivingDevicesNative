#pragma once

namespace UD {
    bool _DamageAV(RE::ActorValueOwner* a_avowner,const RE::ActorValue& a_av, const float& f_dmg, const float& f_min);
    void StartMinigameEffect(RE::BSScript::Internal::VirtualMachine* a_vm, const RE::VMStackID a_stackID, RE::StaticFunctionTag*, RE::Actor *a_actor, float f_mult, float f_stamina, float f_health, float f_magicka);
    void EndMinigameEffect(RE::BSScript::Internal::VirtualMachine* a_vm, const RE::VMStackID a_stackID, RE::StaticFunctionTag*, RE::Actor *a_actor);
    bool IsMinigameEffectOn(RE::BSScript::Internal::VirtualMachine* a_vm, const RE::VMStackID a_stackID, RE::StaticFunctionTag*, RE::Actor *a_actor) ;
    void UpdateMinigameEffectMult(RE::BSScript::Internal::VirtualMachine* a_vm, const RE::VMStackID a_stackID, RE::StaticFunctionTag*, RE::Actor *a_actor, float f_newvalue) ;
    void ToggleMinigameEffect(RE::BSScript::Internal::VirtualMachine* a_vm, const RE::VMStackID a_stackID, RE::StaticFunctionTag*, RE::Actor *a_actor, bool b_toggle) ;
    bool MinigameStatsCheck(RE::BSScript::Internal::VirtualMachine* a_vm, const RE::VMStackID a_stackID, RE::StaticFunctionTag*, RE::Actor *a_actor) ;

    //copied from https://github.com/NoahBoddie/ActorValueGenerator
    struct ActorValueUpdateHook
	{
		static void Patch()
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
            toggle  = true;
		}

		static void Restore()
		{
            SKSE::log::info("ActorValueUpdateHook::Restore()");
            if (ActorValueUpdate.address() != 0) 
            {
                toggle  = false;
            }
		}

		static int32_t ActorValueUpdatePatched( RE::Character* a_actor, RE::ActorValue a_av, float a_unk)
		{
            //RE::ConsoleLog::GetSingleton()->Print(std::format("::ActorValueUpdate health={},stamina={},magicka={},mult={},toggle={}",health,stamina,magicka,mult,toggle).c_str());
            if ((toggle) && (actor == a_actor))
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
		static inline REL::Relocation<decltype(ActorValueUpdatePatched)> ActorValueUpdate;
        static RE::Actor*           actor;
        static std::atomic<float>   health;
        static std::atomic<float>   stamina;
        static std::atomic<float>   magicka;
        static std::atomic<float>   mult;
        static std::atomic<bool>    started;
        static std::atomic<bool>    toggle;
	};
}