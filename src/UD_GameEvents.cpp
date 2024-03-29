#include <UD_GameEvents.h>

namespace UD
{
    inline void _OnGameLoad()
    {
        UD::ReloadLib();
        RandomGenerator::GetSingleton()->Setup();
        PlayerStatus::GetSingleton()->Setup();
        MeterManager::RemoveAll();
        KeywordManager::Reload();
        InventoryHandler::GetSingleton()->Reload();
        ORS::OrgasmManager::GetSingleton()->Setup();
        ActorSlotManager::GetSingleton()->Setup();
        ControlManager::GetSingleton()->Setup();
        PapyrusDelegate::GetSingleton()->Setup();
        MaterialManager::GetSingleton()->Setup();
        Diagnosis::GetSingleton()->Setup();
        LockpickManager::GetSingleton()->Setup();
        //remove effect in case that user reloaded the game without exit
        if (MinigameEffectManager::GetSingleton()->started) MinigameEffectManager::GetSingleton()->RemoveAll();

        UpdateManager::GetSingleton()->CreateUpdateThreads();
    }


    inline void _OnPostPostLoad()
    {
        //LOG("::_OnPostPostLoad called");
    }

    void OnMessageReceived(SKSE::MessagingInterface::Message* a_msg)
    {
        switch(a_msg->type)
        {
            case SKSE::MessagingInterface::kInputLoaded:
                RE::BSInputDeviceManager::GetSingleton()->AddEventSink(KeyEventSink::GetSingleton());
                break;
            case SKSE::MessagingInterface::kPostLoadGame:
            case SKSE::MessagingInterface::kNewGame:
                _OnGameLoad();
                break;
            case SKSE::MessagingInterface::kPostPostLoad:
                _OnPostPostLoad();
                break;
        }
    }
}