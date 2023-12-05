#pragma once

#include <execution>

namespace UD 
{
    using Item = std::pair<RE::TESBoundObject* const, std::pair<int32_t, std::unique_ptr<RE::InventoryEntryData>>>;

    class InventoryHandler
    {
    SINGLETONHEADER(InventoryHandler)
    public:
        void Reload();
        std::vector<RE::TESObjectARMO*> GetInventoryDevices(RE::Actor* a_actor, bool b_worn);
        std::vector<RE::TESObjectARMO*> GetRenderDevices(RE::Actor* a_actor, bool b_worn);
        std::vector<RE::TESObjectARMO*> GetInvisibleDevices(RE::Actor* a_actor);
        RE::TESObjectWEAP* GetSharpestWeapon(RE::Actor* a_actor);
        bool IsSharp(RE::TESObjectWEAP* a_weapon);

        
    private:
        bool _installed = false;
        RE::TESDataHandler*                             _datahandler;
        std::vector<RE::BGSKeyword*>    _idkw;
        std::vector<RE::BGSKeyword*>    _rdkw;
        std::vector<RE::BGSKeyword*>    _inviskw;
    };


    inline std::vector<RE::TESObjectARMO*> GetInventoryDevices(PAPYRUSFUNCHANDLE,RE::Actor* a_actor, bool b_worn)
    {
        return InventoryHandler::GetSingleton()->GetInventoryDevices(a_actor,b_worn);
    }

    inline std::vector<RE::TESObjectARMO*> GetRenderDevices(PAPYRUSFUNCHANDLE,RE::Actor* a_actor, bool b_worn)
    {
        return InventoryHandler::GetSingleton()->GetRenderDevices(a_actor,b_worn);
    }

    inline RE::TESObjectWEAP* GetSharpestWeapon(PAPYRUSFUNCHANDLE,RE::Actor* a_actor)
    {
        if (a_actor == nullptr) return nullptr;

        auto loc_slot = ActorSlotManager::GetSingleton()->GetActorStorage(a_actor);
        return loc_slot->BestWeapon;
    }
}