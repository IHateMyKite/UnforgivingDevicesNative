#include <UD_Animation.h>

namespace UD
{
    int GetActorConstrainsInter(RE::Actor* a_actor)
    {
        if (a_actor == nullptr) return 0x00000000;

        auto loc_devices                = InventoryHandler::GetSingleton()->GetRenderDevices(a_actor, true);
        auto loc_invisibledevices       = InventoryHandler::GetSingleton()->GetInvisibleDevices(a_actor);
        
        int loc_res         = 0x00000000;
        
        loc_res |= ProccessDeviceArray(a_actor,loc_devices);
        loc_res |= ProccessDeviceArray(a_actor,loc_invisibledevices);

        LOG("GetActorConstrainsInter({}) - Result = {:08X}",a_actor->GetName(),loc_res)

        return loc_res;
    }

    inline int ProccessDeviceArray(RE::Actor* a_actor,const std::vector<RE::TESObjectARMO*> &a_array)
    {
        int loc_res = 0x00000000;
        for(auto& it : a_array)
        {
            //check heavy bondage
            static std::vector<RE::BGSKeyword*> loc_hb = std::vector<RE::BGSKeyword*>({KeywordManager::ddhb});
            if (it->HasKeywordInArray(loc_hb,true))
            {
                loc_res |= GetActorHBConstrains(a_actor,it);
            }

            //check hobble skirt
            static std::vector<RE::BGSKeyword*> loc_hobble = std::vector<RE::BGSKeyword*>({KeywordManager::ddhobbleskirt});
            if (it->HasKeywordInArray(loc_hobble,true))
            {
                static std::vector<RE::BGSKeyword*> loc_hobblerelaxed = std::vector<RE::BGSKeyword*>({KeywordManager::ddhobbleskirtrelax,KeywordManager::ddankleshacles});
                if (it->HasKeywordInArray(loc_hobblerelaxed,false)) loc_res |= 0x00000002;
                else loc_res |= 0x00000001;
                continue;
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

    inline int GetActorHBConstrains(RE::Actor* a_actor,RE::TESObjectARMO* a_device)
    {
        for (auto& [kw,id] : KeywordManager::ddhbkwds)
        {
            std::vector<RE::BGSKeyword*> loc_hbkw = std::vector<RE::BGSKeyword*>({kw});
            if (a_device->HasKeywordInArray(loc_hbkw,true))
            {
                return id;
            }
        }
        return 0x00000000;
    }
}