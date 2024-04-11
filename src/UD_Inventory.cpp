#include <UD_Inventory.h>
#include <execution>
#include <UD_Config.h>
#include <UD_Utility.h>
#include <UD_Keywords.h>
#include <UD_Spinlock.h>
#include <UD_ActorSlotManager.h>

SINGLETONBODY(UD::InventoryHandler)

void UD::InventoryHandler::Reload()
{
    if (_installed) return;
    _installed = true;

    //inventory script filter
    _idkw.clear();
    _idkw.push_back(KeywordManager::udinvdevice);

    //render script filder
    _rdkw.clear();
    _rdkw.push_back(KeywordManager::udrendevice);

    _inviskw.clear();
    _inviskw.push_back(KeywordManager::udinvhb);
    _inviskw.push_back(KeywordManager::udinvhs);

    LOG("InventoryHandler::Reload called")
}

std::vector<RE::TESObjectARMO*> UD::InventoryHandler::GetInventoryDevices(RE::Actor* a_actor, bool b_worn)
{
    LOG("GetInventoryDevices called")
                
    if (a_actor == nullptr) return std::vector<RE::TESObjectARMO*>();

    std::vector<RE::TESObjectARMO*> loc_res;

    if (b_worn)
    {
        auto loc_visitor = WornVisitor([this,&loc_res](RE::InventoryEntryData* a_entry)
        {
            #undef GetObject
            auto loc_object = a_entry->GetObject();
            RE::TESObjectARMO* loc_armor = nullptr;
            if (loc_object != nullptr && loc_object->IsArmor()) loc_armor = static_cast<RE::TESObjectARMO*>(loc_object);

            if (loc_armor != nullptr && loc_armor->HasKeywordInArray(_idkw,false))
            {
                loc_res.push_back(loc_armor);
            }
            return RE::BSContainer::ForEachResult::kContinue;
        });
        a_actor->GetInventoryChanges()->VisitWornItems(loc_visitor.AsNativeVisitor());
    }
    else
    {
        RE::Actor::InventoryItemMap loc_inv = a_actor->GetInventory();

        Spinlock loc_lock;

        std::for_each(std::execution::seq,loc_inv.begin(),loc_inv.end(),[this,&loc_lock,&loc_res](Item& a_obj)
        {
            if (a_obj.first && a_obj.first->As<RE::TESObjectARMO>() && a_obj.first->HasKeywordInArray(_idkw,false)) 
            {
                //UniqueLock lock(loc_lock);
                loc_res.push_back(a_obj.first->As<RE::TESObjectARMO>());
            }
        });
    }

    return loc_res;
}

std::vector<RE::TESObjectARMO*> UD::InventoryHandler::GetRenderDevices(RE::Actor* a_actor, bool b_worn)
{
    LOG("GetRenderDevices called")
    if (a_actor == nullptr) return std::vector<RE::TESObjectARMO*>();
                
    std::vector<RE::TESObjectARMO*> loc_res;

    if (b_worn)
    {
        auto loc_visitor = WornVisitor([this,&loc_res](RE::InventoryEntryData* a_entry)
        {
            #undef GetObject
            auto loc_object = a_entry->GetObject();
            RE::TESObjectARMO* loc_armor = nullptr;

            LOG("WornForm = {}",a_entry ? a_entry->GetDisplayName() : "NONE")

            if (loc_object != nullptr && loc_object->IsArmor()) loc_armor = static_cast<RE::TESObjectARMO*>(loc_object);

            if (loc_armor != nullptr && loc_armor->HasKeywordInArray(_rdkw,false))
            {
                loc_res.push_back(loc_armor);
            }
            return RE::BSContainer::ForEachResult::kContinue;
        });
        a_actor->GetInventoryChanges()->VisitWornItems(loc_visitor.AsNativeVisitor());
    }
    else
    {
        RE::Actor::InventoryItemMap loc_inv = a_actor->GetInventory();

        Spinlock loc_lock;

        std::for_each(std::execution::seq,loc_inv.begin(),loc_inv.end(),[this,&loc_lock,&loc_res](Item& a_obj)
        {
            if (a_obj.first && a_obj.first->As<RE::TESObjectARMO>() && a_obj.first->HasKeywordInArray(_rdkw,false)) 
            {
                //UniqueLock lock(loc_lock);
                loc_res.push_back(a_obj.first->As<RE::TESObjectARMO>());
            }
        });
    }
    return loc_res;
}

std::vector<RE::TESObjectARMO*> UD::InventoryHandler::GetInvisibleDevices(RE::Actor* a_actor)
{
    LOG("GetInvisibleDevices called")
    if (a_actor == nullptr) return std::vector<RE::TESObjectARMO*>();
                
    std::vector<RE::TESObjectARMO*> loc_res;
    auto loc_visitor = WornVisitor([this,&loc_res](RE::InventoryEntryData* a_entry)
    {
        #undef GetObject
        auto loc_object = a_entry->GetObject();
        RE::TESObjectARMO* loc_armor = nullptr;
        if (loc_object != nullptr && loc_object->IsArmor()) loc_armor = static_cast<RE::TESObjectARMO*>(loc_object);

        if (loc_armor != nullptr && loc_armor->HasKeywordInArray(_inviskw,false))
        {
            loc_res.push_back(loc_armor);
        }
        return RE::BSContainer::ForEachResult::kContinue;
    });
    a_actor->GetInventoryChanges()->VisitWornItems(loc_visitor.AsNativeVisitor());
    return loc_res;
}

RE::TESObjectWEAP* UD::InventoryHandler::GetSharpestWeapon(RE::Actor* a_actor)
{
    LOG("InventoryHandler::GetSharpestWeapon({}) called",a_actor ? a_actor->GetName() : "NONE")

    RE::TESObjectWEAP* loc_result = nullptr;

    RE::Actor::InventoryItemMap loc_inv = a_actor->GetInventory();

    Spinlock loc_lock;

    std::for_each(std::execution::seq,loc_inv.begin(),loc_inv.end(),[this,&loc_lock,&loc_result](Item& a_obj)
    {
        RE::TESObjectWEAP* loc_weapon = a_obj.first ? a_obj.first->As<RE::TESObjectWEAP>() : nullptr;
        if (loc_weapon && IsSharp(loc_weapon)) 
        {
            //UniqueLock lock(loc_lock);
            if (loc_result == nullptr) loc_result = loc_weapon;
            else
            {
                if (loc_weapon->GetAttackDamage() > loc_result->GetAttackDamage()) loc_result = loc_weapon;
            }
        }
    });

    return loc_result;
}

bool UD::InventoryHandler::IsSharp(RE::TESObjectWEAP* a_weapon)
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

RE::TESObjectWEAP* UD::GetSharpestWeapon(PAPYRUSFUNCHANDLE, RE::Actor* a_actor)
{
    if (a_actor == nullptr) return nullptr;

    auto loc_slot = ActorSlotManager::GetSingleton()->GetActorStorage(a_actor);
    return loc_slot->BestWeapon;
}
