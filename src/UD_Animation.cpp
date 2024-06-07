#include <UD_Animation.h>
#include <UD_Utility.h>
#include <UD_Config.h>
#include <UD_Inventory.h>
#include <UD_Keywords.h>

SINGLETONBODY(UD::AnimationManager);

int UD::AnimationManager::GetActorConstrainsInter(RE::Actor* a_actor) const
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

inline bool UD::AnimationManager::CheckWeaponDisabled(RE::Actor* a_actor) const
{
    if (a_actor == nullptr || a_actor->IsPlayerRef()) return false;
    Utils::UniqueLock lock(_SaveLock);
    return (std::find(_weapondisabled.begin(),_weapondisabled.end(),a_actor->GetHandle().native_handle()) != _weapondisabled.end());
}

void UD::AnimationManager::DisableWeapons(RE::Actor* a_actor, bool a_state)
{
    LOG("DisableWeapons({},{}) called",a_actor ? a_actor->GetName() : "NONE", a_state)
    if (a_actor == nullptr || a_actor->IsPlayerRef()) return;
    
    bool loc_disabled = CheckWeaponDisabled(a_actor);

    Utils::UniqueLock lock(_SaveLock);
    if (!loc_disabled && a_state)
    {
        _weapondisabled.push_back(a_actor->GetHandle().native_handle());
    }
    else if (loc_disabled && !a_state)
    {
        Handle loc_handle = a_actor->GetHandle().native_handle();
        (void)_weapondisabled.erase(std::find(_weapondisabled.begin(),_weapondisabled.end(),loc_handle));
    }
}

inline int UD::AnimationManager::ProccessDeviceArray(RE::Actor* a_actor,const std::vector<RE::TESObjectARMO*> &a_array) const
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

inline int UD::AnimationManager::GetActorHBConstrains(RE::Actor* a_actor,RE::TESObjectARMO* a_device) const
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

void UD::AnimationManager::Setup()
{
    if (!_init)
    {
        _init = true;
        REL::Relocation<std::uintptr_t> vtbl_player{RE::Character::VTABLE[0]};
        DrawWeaponMagicHands_old = vtbl_player.write_vfunc(REL::Module::GetRuntime() != REL::Module::Runtime::VR ? 0x0A6 : 0x0A8, DrawWeaponMagicHands);
    }
}

void UD::AnimationManager::Reload()
{
    Utils::UniqueLock lock(_SaveLock);
    _weapondisabled.clear();
}

void UD::AnimationManager::DrawWeaponMagicHands(RE::Actor* a_actor, bool a_draw)
{
    static const bool loc_boundcombatnpc = Config::GetSingleton()->GetVariable<bool>("Combat.bNPCBoundCombat",true);

    // Check if actor weapons are disabled
    if (a_draw)
    {
        if (AnimationManager::GetSingleton()->CheckWeaponDisabled(a_actor))
        {
            LOG("ControlManager::DrawWeaponMagicHands({}) - actors weapons are disabled and because of that cant draw weapon",a_actor ? a_actor->GetName() : "NONE")
            return;
        }
    }


    if (a_draw && (IsAnimating(a_actor) || (!loc_boundcombatnpc && ActorIsBound(a_actor))))
    {
        
        LOG("ControlManager::DrawWeaponMagicHands({}) - actor is animating/bound and because of that cant draw weapon",a_actor ? a_actor->GetName() : "NONE")
        return;
    } 
    else
    {
        DrawWeaponMagicHands_old(a_actor,a_draw);
    }
}