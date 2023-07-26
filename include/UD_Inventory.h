#pragma once

namespace UD 
{
    static class InventoryHandler
    {
        public:
            static void Reload()
            {
                //inventory script filter
                invfilter.first.clear();
                invfilter.first.push_back(KeywordManager::udinvdevice);
                invfilter.second = true;

                //render script filder
                renfilter.first.clear();
                renfilter.first.push_back(KeywordManager::udrendevice);
                renfilter.second = true;

                invisiblefilter.first.clear();
                invisiblefilter.first.push_back(KeywordManager::udinvhb);
                invisiblefilter.first.push_back(KeywordManager::udinvhs);
                invisiblefilter.second = false;

                UDSKSELOG("InventoryHandler::Reload called")
            }

            inline static std::vector<RE::TESForm*> GetInventoryDevices(RE::Actor* a_actor, bool b_worn)
            {
                if (a_actor == nullptr) return std::vector<RE::TESForm*>();

                RE::Actor::InventoryItemMap     loc_inv = a_actor->GetInventory(FilterInvDevices);

                std::vector<RE::TESForm*> loc_res;

                for(auto&& it : loc_inv)
                {
                    if ((!b_worn) || (it.second.second->IsWorn())) 
                    {
                        loc_res.push_back(it.first);
                    }
                }
                return loc_res;
            }

            inline static std::vector<RE::TESForm*> GetRenderDevices(RE::Actor* a_actor, bool b_worn)
            {
                if (a_actor == nullptr) return std::vector<RE::TESForm*>();
                
                RE::Actor::InventoryItemMap     loc_inv = a_actor->GetInventory(FilterRenDevices);

                std::vector<RE::TESForm*> loc_res;

                for(auto&& it : loc_inv)
                {
                    if ((!b_worn) || (it.second.second->IsWorn())) 
                    {
                        loc_res.push_back(it.first);
                    }
                }
                return loc_res;
            }

            inline static std::vector<RE::TESForm*> GetInvisibleDevices(RE::Actor* a_actor, bool b_worn)
            {
                if (a_actor == nullptr) return std::vector<RE::TESForm*>();

                RE::Actor::InventoryItemMap     loc_inv = a_actor->GetInventory(FilterInvisibleDevices);

                std::vector<RE::TESForm*> loc_res;

                for(auto&& it : loc_inv)
                {
                    if ((!b_worn) || (it.second.second->IsWorn())) 
                    {
                        loc_res.push_back(it.first);
                    }
                }
                return loc_res;
            }

            inline static RE::TESObjectWEAP* GetSharpestWeapon(RE::Actor* a_actor)
            {
                if (a_actor == nullptr) return nullptr;

                RE::Actor::InventoryItemMap loc_weapons = a_actor->GetInventory(FilterWeapons);
                RE::TESObjectWEAP* loc_result = nullptr;

                for(auto&& it : loc_weapons)
                {
                    RE::TESObjectWEAP* loc_weapon = static_cast<RE::TESObjectWEAP*>(it.first);
                    if (IsSharp(loc_weapon)) 
                    {
                        if (loc_result == nullptr) loc_result = loc_weapon;
                        else
                        {
                            if (loc_weapon->GetAttackDamage() > loc_result->GetAttackDamage()) loc_result = loc_weapon;
                        }
                    }
                }

                return loc_result;
            }

            static bool IsSharp(RE::TESObjectWEAP* a_weapon)
            {
                if (a_weapon == nullptr) return false;
                
                RE::WEAPON_TYPE loc_type = a_weapon->GetWeaponType();
                switch (loc_type)
                {
                case RE::WEAPON_TYPE::kOneHandDagger:
                case RE::WEAPON_TYPE::kOneHandSword:
                case RE::WEAPON_TYPE::kTwoHandSword:
                case RE::WEAPON_TYPE::kOneHandAxe:
                case RE::WEAPON_TYPE::kTwoHandAxe:
                    return true;
                    break;
                default:
                    return false;
                }
            }

            inline static bool FilterInvDevices(RE::TESBoundObject& a_obj)
            {
                return a_obj.HasKeywordInArray(invfilter.first,invfilter.second);
            }
            inline static bool FilterRenDevices(RE::TESBoundObject& a_obj)
            {
                return a_obj.HasKeywordInArray(renfilter.first,renfilter.second);
            }
            inline static bool FilterWeapons(RE::TESBoundObject& a_obj)
            {
                return a_obj.IsWeapon();
            }
            inline static bool FilterInvisibleDevices(RE::TESBoundObject& a_obj)
            {
                return a_obj.HasKeywordInArray(invisiblefilter.first,invisiblefilter.second);
            }

            static RE::TESDataHandler* datahandler;

            static std::pair<std::vector<RE::BGSKeyword*>,bool> invfilter;
            static std::pair<std::vector<RE::BGSKeyword*>,bool> renfilter;
            static std::pair<std::vector<RE::BGSKeyword*>,bool> invisiblefilter;
    };


    inline std::vector<RE::TESForm*> GetInventoryDevices(PAPYRUSFUNCHANDLE,RE::Actor* a_actor, bool b_worn)
    {
        return InventoryHandler::GetInventoryDevices(a_actor,b_worn);
    }

    inline std::vector<RE::TESForm*> GetRenderDevices(PAPYRUSFUNCHANDLE,RE::Actor* a_actor, bool b_worn)
    {
        return InventoryHandler::GetRenderDevices(a_actor,b_worn);
    }

    inline RE::TESObjectWEAP* GetSharpestWeapon(PAPYRUSFUNCHANDLE,RE::Actor* a_actor)
    {
        return InventoryHandler::GetSharpestWeapon(a_actor);
    }
}