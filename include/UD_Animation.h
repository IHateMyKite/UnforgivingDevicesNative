#pragma once

#include <UD_Spinlock.h>

namespace UD 
{
    typedef uint8_t DisableCounter;

    class AnimationManager
    {
    SINGLETONHEADER(AnimationManager);
    public:
        void Setup();
        void Reload();

        int GetActorHBConstrains(RE::Actor* a_actor,RE::TESObjectARMO* a_device) const;
        int ProccessDeviceArray(RE::Actor* a_actor,const std::vector<RE::TESObjectARMO*> &a_array) const;
        int GetActorConstrainsInter(RE::Actor* a_actor) const;

        bool CheckWeaponDisabled(RE::Actor* a_actor);
        void DisableWeapons(RE::Actor* a_actor, bool a_state);
    private:
        static void DrawWeaponMagicHands(RE::Actor* a_actor, bool a_draw);
        inline static REL::Relocation<decltype(DrawWeaponMagicHands)> DrawWeaponMagicHands_old;
    private:
        bool _init = false;
        ActorMap<DisableCounter> _weapondisabled;
        mutable Utils::Spinlock _SaveLock;
    };

    // ==========================================
    // ===         PAPYRUS FUNCTIONS          ===
    // ==========================================

    inline int GetActorConstrains(PAPYRUSFUNCHANDLE,RE::Actor* a_actor)
    {
        return AnimationManager::GetSingleton()->GetActorConstrainsInter(a_actor);
    }
    inline bool CheckWeaponDisabled(PAPYRUSFUNCHANDLE,RE::Actor* a_actor)
    {
        return AnimationManager::GetSingleton()->CheckWeaponDisabled(a_actor);
    }
    inline void DisableWeapons(PAPYRUSFUNCHANDLE,RE::Actor* a_actor, bool a_state)
    {
        AnimationManager::GetSingleton()->DisableWeapons(a_actor,a_state);
    }
}