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
            struct ActorControl {
                ActorControl();
                ActorControl(float f_stamina, float f_health, float f_magicka,float f_mult, bool b_toggle);
                ActorControl& operator=(const ActorControl& source);
                std::atomic<float>   stamina;
                std::atomic<float>   health;
                std::atomic<float>   magicka;
                std::atomic<float>   mult;
                std::atomic<bool>    toggle;
            };

		    static void Patch();
		    static int32_t ActorValueUpdatePatched( RE::Character* a_actor, RE::ActorValue a_av, float a_unk);
            static inline REL::Relocation<decltype(ActorValueUpdatePatched)> ActorValueUpdate;
            //register actor for effect
            static void RegisterActor(RE::Actor *a_actor, float f_mult, float f_stamina, float f_health, float f_magicka, bool b_toggle);
            
            //unregister actor for effect
            static void RemoveActor(RE::Actor *a_actor);
            
            //returns true if actor is registered
            static bool IsRegistered(RE::Actor *a_actor);

            static void RemoveAll(void);
            //get actor control
            static ActorControl* GetActorControl(RE::Actor *a_actor);
        public:
            static std::atomic<bool>    started;
            static const std::atomic<float>   mult;

        private:
            static std::map<RE::Actor*,ActorControl> _actormap;
	};
}