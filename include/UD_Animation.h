#pragma once

#include <UD_H.h>

namespace UD 
{
    extern inline int GetActorHBConstrains(RE::Actor* a_actor,RE::TESForm* a_device);
    extern inline int ProccessDeviceArray(RE::Actor* a_actor,const std::vector<RE::TESForm*> &a_array);
    extern inline int GetActorConstrainsInter(RE::Actor* a_actor);

    inline int GetActorConstrains(PAPYRUSFUNCHANDLE,RE::Actor* a_actor)
    {
        const auto loc_storage = ActorSlotManager::GetSingleton()->GetActorStorage(a_actor);
        return loc_storage ? loc_storage->Constrains : GetActorConstrainsInter(a_actor); //GetActorConstrainsInter(a_actor);
    }
}