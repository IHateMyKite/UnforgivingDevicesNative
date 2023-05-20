#pragma once

namespace UD {
    bool _DamageAV(RE::ActorValueOwner* a_avowner,const RE::ActorValue& a_av, const float& f_dmg, const float& f_min);
    void StartMinigameEffect(RE::BSScript::Internal::VirtualMachine* a_vm, const RE::VMStackID a_stackID, RE::StaticFunctionTag*, RE::Actor *a_actor, float f_mult, float f_stamina, float f_health, float f_magicka, bool b_toggle);
    void EndMinigameEffect(RE::BSScript::Internal::VirtualMachine* a_vm, const RE::VMStackID a_stackID, RE::StaticFunctionTag*, RE::Actor *a_actor);
    bool IsMinigameEffectOn(RE::BSScript::Internal::VirtualMachine* a_vm, const RE::VMStackID a_stackID, RE::StaticFunctionTag*, RE::Actor *a_actor);
    void UpdateMinigameEffectMult(RE::BSScript::Internal::VirtualMachine* a_vm, const RE::VMStackID a_stackID, RE::StaticFunctionTag*, RE::Actor *a_actor, float f_newvalue);
    void ToggleMinigameEffect(RE::BSScript::Internal::VirtualMachine* a_vm, const RE::VMStackID a_stackID, RE::StaticFunctionTag*, RE::Actor *a_actor, bool b_toggle);
    bool MinigameStatsCheck(RE::BSScript::Internal::VirtualMachine* a_vm, const RE::VMStackID a_stackID, RE::StaticFunctionTag*, RE::Actor *a_actor);
    void MinigameEffectUpdateHealth(RE::BSScript::Internal::VirtualMachine* a_vm, const RE::VMStackID a_stackID, RE::StaticFunctionTag*, RE::Actor *a_actor, float f_health);
    void MinigameEffectUpdateStamina(RE::BSScript::Internal::VirtualMachine* a_vm, const RE::VMStackID a_stackID, RE::StaticFunctionTag*, RE::Actor *a_actor, float f_stamina);
    void MinigameEffectUpdateMagicka(RE::BSScript::Internal::VirtualMachine* a_vm, const RE::VMStackID a_stackID, RE::StaticFunctionTag*, RE::Actor *a_actor, float f_magicka);
    
    //copied from https://github.com/NoahBoddie/ActorValueGenerator
    struct ActorValueUpdateHook
	{
        public:
		    static void Patch();
		    static void Restore();
		    static int32_t ActorValueUpdatePatched( RE::Character* a_actor, RE::ActorValue a_av, float a_unk);
            static inline REL::Relocation<decltype(ActorValueUpdatePatched)> ActorValueUpdate;

        public:
            static RE::Actor*           actor;
            static std::atomic<float>   health;
            static std::atomic<float>   stamina;
            static std::atomic<float>   magicka;
            static std::atomic<float>   mult;
            static std::atomic<bool>    started;
            static std::atomic<bool>    toggle;
	};
}