#pragma once

#include <UD_H.h>

namespace UD 
{
    inline int GetActorHBConstrains(RE::Actor* a_actor,RE::TESForm* a_device);
    inline int ProccessDeviceArray(RE::Actor* a_actor,const std::vector<RE::TESForm*> &a_array);
    inline int GetActorConstrainsInter(RE::Actor* a_actor);

    inline int GetActorConstrains(PAPYRUSFUNCHANDLE,RE::Actor* a_actor)
    {
/*        if (a_actor->IsPlayer()) return UpdateManager::PLAYERCONSTRAINS;
        else*/ return GetActorConstrainsInter(a_actor);
    }

    inline int GetActorConstrainsInter(RE::Actor* a_actor)
    {
        auto loc_devices                = InventoryHandler::GetRenderDevices(a_actor, true);
        auto loc_invisibledevices       = InventoryHandler::GetInvisibleDevices(a_actor, true);
        
        int loc_res         = 0x00000000;
        
        loc_res |= ProccessDeviceArray(a_actor,loc_devices);
        loc_res |= ProccessDeviceArray(a_actor,loc_invisibledevices);

        return loc_res;
    }

    inline int ProccessDeviceArray(RE::Actor* a_actor,const std::vector<RE::TESForm*> &a_array)
    {
        int loc_res = 0x00000000;
        for(auto& it : a_array)
        {
            //check hobble skirt
            static std::vector<RE::BGSKeyword*> loc_hobble = std::vector<RE::BGSKeyword*>({KeywordManager::ddhobbleskirt});
            if (it->HasKeywordInArray(loc_hobble,true))
            {
                static std::vector<RE::BGSKeyword*> loc_hobblerelaxed = std::vector<RE::BGSKeyword*>({KeywordManager::ddhobbleskirtrelax,KeywordManager::ddankleshacles});
                if (it->HasKeywordInArray(loc_hobblerelaxed,false)) loc_res |= 0x00000002;
                else loc_res |= 0x00000001;
                continue;
            }

            //check heavy bondage
            static std::vector<RE::BGSKeyword*> loc_hb = std::vector<RE::BGSKeyword*>({KeywordManager::ddhb});
            if (it->HasKeywordInArray(loc_hb,true))
            {
                loc_res |= GetActorHBConstrains(a_actor,it);
            }

            //check mittens
            static std::vector<RE::BGSKeyword*> loc_mittens = std::vector<RE::BGSKeyword*>({KeywordManager::ddmittens});
            if (it->HasKeywordInArray(loc_mittens,true))
            {
                loc_res |= 0x00000100;
                continue;
            }

            //check gag
            static std::vector<RE::BGSKeyword*> loc_gag = std::vector<RE::BGSKeyword*>({KeywordManager::ddgag});
            if (it->HasKeywordInArray(loc_gag,true))
            {
                loc_res |= 0x00000800;
                continue;
            }
        }
        return loc_res;
    }

    inline int GetActorHBConstrains(RE::Actor* a_actor,RE::TESForm* a_device)
    {
        for (auto& it2 : KeywordManager::ddhbkwds)
        {
            std::vector<RE::BGSKeyword*> loc_hbkw = std::vector<RE::BGSKeyword*>({it2.first});
            if (a_device->HasKeywordInArray(loc_hbkw,true))
            {
                return it2.second;
            }
        }
        return 0x00000000;
    }
}