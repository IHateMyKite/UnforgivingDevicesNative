#pragma once

#include <UD_Spinlock.h>

namespace boost::json
{
    class object;
    class value;
}

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

        std::vector<std::string> GetAnimationsFromJSON(std::string a_def, std::vector<RE::Actor*> a_actors, int a_constraintsORA1, int a_constraintsORA2);
    private:
        static void DrawWeaponMagicHands(RE::Actor* a_actor, bool a_draw);
        inline static REL::Relocation<decltype(DrawWeaponMagicHands)> DrawWeaponMagicHands_old;
        bool _CheckConstraints(boost::json::object& a_obj, std::string a_ObjPath, int a_ActorConstraints) const;

        boost::json::value RecursiveFind(boost::json::object& a_obj,std::string a_path) const;

    private:
        bool _init = false;
        ActorMap<DisableCounter> _weapondisabled;
        std::unordered_map<std::string,std::shared_ptr<boost::json::value>> _jsoncache;
        mutable Utils::Spinlock _SaveLock;
    };

    // ==========================================
    // ===         PAPYRUS FUNCTIONS          ===
    // ==========================================

    inline int GetActorConstrains(PAPYRUSFUNCHANDLE,RE::Actor* a_actor)
    {
        return AnimationManager::GetSingleton()->GetActorConstrainsInter(a_actor);
    }
    inline std::vector<std::string> GetAnimationsFromJSON(PAPYRUSFUNCHANDLE,std::string a_def, std::vector<RE::Actor*> a_actors, int a_constraintsORA1, int a_constraintsORA2)
    {
        return AnimationManager::GetSingleton()->GetAnimationsFromJSON(a_def,a_actors,a_constraintsORA1,a_constraintsORA2);
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