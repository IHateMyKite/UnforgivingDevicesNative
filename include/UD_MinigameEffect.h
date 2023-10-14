#pragma once
#include <Windows.h>

namespace UD {
    inline bool _DamageAV(RE::ActorValueOwner* a_avowner,const RE::ActorValue& a_av, const float& f_dmg, const float& f_min);
    void StartMinigameEffect(PAPYRUSFUNCHANDLE, RE::Actor *a_actor, float f_mult, float f_stamina, float f_health, float f_magicka, bool b_toggle);
    void EndMinigameEffect(PAPYRUSFUNCHANDLE, RE::Actor *a_actor);
    bool IsMinigameEffectOn(PAPYRUSFUNCHANDLE, RE::Actor *a_actor);
    void UpdateMinigameEffectMult(PAPYRUSFUNCHANDLE, RE::Actor *a_actor, float f_newvalue);
    void ToggleMinigameEffect(PAPYRUSFUNCHANDLE, RE::Actor *a_actor, bool b_toggle);
    bool MinigameStatsCheck(PAPYRUSFUNCHANDLE, RE::Actor *a_actor, bool a_stamina, bool a_health, bool a_magicka);
    void MinigameEffectSetHealth(PAPYRUSFUNCHANDLE, RE::Actor *a_actor, float f_health);
    void MinigameEffectSetStamina(PAPYRUSFUNCHANDLE, RE::Actor *a_actor, float f_stamina);
    void MinigameEffectSetMagicka(PAPYRUSFUNCHANDLE, RE::Actor *a_actor, float f_magicka);
    void MinigameEffectUpdateHealth(PAPYRUSFUNCHANDLE, RE::Actor *a_actor, float f_health);
    void MinigameEffectUpdateStamina(PAPYRUSFUNCHANDLE, RE::Actor *a_actor, float f_stamina);
    void MinigameEffectUpdateMagicka(PAPYRUSFUNCHANDLE, RE::Actor *a_actor, float f_magicka);

    struct MinigameEffectManager
	{
        SINGLETONHEADER(MinigameEffectManager)
        public:
            struct ActorControl {
                inline ActorControl();
                inline ActorControl(float f_stamina, float f_health, float f_magicka,float f_mult, bool b_toggle);
                inline ActorControl& operator=(const ActorControl& source);
                std::atomic<float>   stamina;
                std::atomic<float>   health;
                std::atomic<float>   magicka;
                std::atomic<float>   mult;
                std::atomic<bool>    toggle;
            };

            //register actor for effect
            void RegisterActor(RE::Actor *a_actor, float f_mult, float f_stamina, float f_health, float f_magicka, bool b_toggle);
            
            //unregister actor for effect
            inline void RemoveActor(RE::Actor *a_actor);
            
            //returns true if actor is registered
            inline bool IsRegistered(RE::Actor *a_actor);

            void RemoveAll(void);

            //get actor control
            inline ActorControl* GetActorControl(RE::Actor *a_actor);

            void UpdateMinigameEffect(RE::Actor* a_actor, const float& a_delta);
            void UpdateMeters(RE::Actor* a_actor, const float& a_delta);

        public:
            std::atomic<bool>            started;
        private:
            std::map<RE::Actor*,ActorControl> _actormap;
	};
}