#pragma once

namespace UD
{
    struct ActorStorage
    {
        int Constrains;
    };


    class ActorSlotManager
    {
    SINGLETONHEADER(ActorSlotManager)
    public:
        void Setup();

        void Update();


        std::vector<RE::Actor*> GetRegisteredActors();

        ActorStorage GetActorStorage(RE::Actor* a_actor);

        bool RegisterSlotQuest(RE::TESQuest* a_quest);
    private:
        mutable std::mutex  _lock;

        std::vector<RE::TESQuest*> _slotquests;

        std::unordered_map<RE::Actor*,ActorStorage>* _slots = nullptr;
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