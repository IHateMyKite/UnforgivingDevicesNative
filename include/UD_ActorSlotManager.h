#pragma once

#include <UD_Spinlock.h>

namespace UD
{
    struct ActorStorage
    {
        RE::TESObjectWEAP* BestWeapon = nullptr;
    };

    class ActorSlotManager
    {
    SINGLETONHEADER(ActorSlotManager)
    public:
        void Setup();

        void Update();

        std::vector<uint32_t> GetValidActors();

        std::vector<RE::Actor*> GetRegisteredActors();

        ActorStorage* GetActorStorage(RE::Actor* a_actor);

        bool RegisterSlotQuest(RE::TESQuest* a_quest);
    private:
        mutable Spinlock  _lock;

        std::vector<RE::TESQuest*> _slotquests;

        std::unordered_map<RE::Actor*,ActorStorage>* _slots = nullptr;
        std::vector<uint32_t> _closeactors;

        bool _installed = false;

        void ValidateAliases();
    };

    inline bool RegisterSlotQuest(PAPYRUSFUNCHANDLE,RE::TESQuest* a_quest)
    {
        return ActorSlotManager::GetSingleton()->RegisterSlotQuest(a_quest);
    }
    inline std::vector<RE::Actor*> GetRegisteredActors(PAPYRUSFUNCHANDLE)
    {
        return ActorSlotManager::GetSingleton()->GetRegisteredActors();
    }
}