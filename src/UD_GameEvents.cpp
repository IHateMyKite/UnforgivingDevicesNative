#include <UD_GameEvents.h>

namespace UD
{
    inline void _OnGameLoad()
    {
        UDSKSELOG("::_OnGameLoad called, effect started={}",ActorValueUpdateHook::started);

        //remove effect in case that user reloaded the game without exit
        if (!ActorValueUpdateHook::started) ActorValueUpdateHook::Patch(); 
        else ActorValueUpdateHook::RemoveAll();

        MeterManager::RemoveAll();
        KeywordManager::Reload();
        InventoryHandler::Reload();
    }

    inline void _OnPostPostLoad()
    {
        UDSKSELOG("::_OnPostPostLoad called");
    }

    void OnMessageReceived(SKSE::MessagingInterface::Message* a_msg)
    {
        switch(a_msg->type)
        {
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