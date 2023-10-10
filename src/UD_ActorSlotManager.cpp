#include <UD_ActorSlotManager.h>

SINGLETONBODY(UD::ActorSlotManager)

void UD::ActorSlotManager::Setup()
{
    if (_installed) return;

    RE::TESDataHandler* loc_datahandler = RE::TESDataHandler::GetSingleton();
    if (loc_datahandler)
    {
        _slotquest = reinterpret_cast<RE::TESQuest*>(loc_datahandler->LookupForm(0x14E7EB,"UnforgivingDevices.esp"));
        _installed = true;
        _slots = new std::unordered_map<RE::Actor*,ActorStorage>();
    }
}

void UD::ActorSlotManager::Update()
{
    ValidateAliases();

    for (auto&& it : *_slots)
    {
        UDSKSELOG("Updating {}",it.first->GetName())
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

UD::ActorStorage UD::ActorSlotManager::GetActorStorage(RE::Actor* a_actor)
{
    return (*_slots)[a_actor];
}

void UD::ActorSlotManager::ValidateAliases()
{
    auto loc_aliases = _slotquest->refAliasMap;

    std::unordered_map<RE::Actor*,ActorStorage>* loc_slots = new std::unordered_map<RE::Actor*,ActorStorage>();;

    for (auto&& it : loc_aliases)
    {
        RE::Actor* loc_actor = reinterpret_cast<RE::Actor*>(it.second.get().get());
        if (loc_actor == nullptr) continue;  //skip empty slot

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
    delete _slots;
    _slots = loc_slots;
}
