#include <UD_ActorSlotManager.h>

SINGLETONBODY(UD::ActorSlotManager)

void UD::ActorSlotManager::Setup()
{
    if (_installed) return;
    _installed = true;
    //RE::TESDataHandler* loc_datahandler = RE::TESDataHandler::GetSingleton();
    //if (loc_datahandler)
    //{
    //    _slotquests.push_back(reinterpret_cast<RE::TESQuest*>(loc_datahandler->LookupForm(0x14E7EB,"UnforgivingDevices.esp")));
    //    _installed = true;
    //    _slots = new std::unordered_map<RE::Actor*,ActorStorage>();
    //}
}

void UD::ActorSlotManager::Update()
{
    std::unique_lock lock(_lock);
    ValidateAliases();

    if (_slots == nullptr) return;

    for (auto&& it : *_slots)
    {
        //UDSKSELOG("Updating {}",it.first->GetName())
        it.second.Constrains = GetActorConstrainsInter(it.first);
    }
}

std::vector<RE::Actor*> UD::ActorSlotManager::GetRegisteredActors()
{
    if (_slots == nullptr) return std::vector<RE::Actor*>();
    
    std::vector<RE::Actor*> loc_result;

    for (auto&& it : *_slots)
    {
        loc_result.push_back(it.first);
    }
    return loc_result;
}

UD::ActorStorage* UD::ActorSlotManager::GetActorStorage(RE::Actor* a_actor)
{
    return (_slots && a_actor) ? &(*_slots)[a_actor] : nullptr;
}

bool UD::ActorSlotManager::RegisterSlotQuest(RE::TESQuest* a_quest)
{
    if (a_quest == nullptr) return false;
    std::unique_lock lock(_lock);
    if (std::find(_slotquests.begin(),_slotquests.end(),a_quest) == _slotquests.end())
    {
        _slotquests.push_back(a_quest);
        UDSKSELOG("ActorSlotManager::RegisterSlotQuest({}) - Total number = {}",a_quest->GetName(),_slotquests.size())
        //ValidateAliases();
        return true;
    }
    else
    {
        return false;
    }
}

void UD::ActorSlotManager::ValidateAliases()
{
    std::unordered_map<RE::Actor*,ActorStorage>* loc_slots = new std::unordered_map<RE::Actor*,ActorStorage>();

    for (auto&& it1 : _slotquests)
    {
        auto loc_aliases = it1->refAliasMap;
        for (auto&& it2 : loc_aliases)
        {
            RE::Actor* loc_actor = reinterpret_cast<RE::Actor*>(it2.second.get().get());
            if (loc_actor == nullptr) continue;  //skip empty slot

            //UDSKSELOG("ActorSlotManager::ValidateAliases() - Checking actor {}",loc_actor->GetName())

            if (_slots && (_slots->find(loc_actor) == _slots->end()))
            {
                //exist, copy it
                memcpy(&(*loc_slots)[loc_actor],&(*_slots)[loc_actor],sizeof(ActorStorage));
            }
            else
            {
                (*loc_slots)[loc_actor] = ActorStorage();
            }
        }
    }
    delete _slots;
    _slots = loc_slots;

}
