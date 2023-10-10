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

    private:
        RE::TESQuest* _slotquest;
        std::unordered_map<RE::Actor*,ActorStorage>* _slots;
        bool _installed = false;

        void ValidateAliases();
    };
}