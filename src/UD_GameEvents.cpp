#include <UD_GameEvents.h>

namespace UD
{
    inline void _OnGameLoad()
    {
        SKSE::log::info("::_OnGameLoad called, effect started={}",ActorValueUpdateHook::started);
        //_CheckUI();
        //remove effect in case that user reloaded the game without exit
        if (ActorValueUpdateHook::started) ActorValueUpdateHook::RemoveAll();
        else ActorValueUpdateHook::Patch();

        MeterManager::RemoveAll();
    }

    inline void _OnPostPostLoad()
    {
        SKSE::log::info("::_OnPostPostLoad called");
    }

    void OnMessageReceived(SKSE::MessagingInterface::Message* a_msg)
    {
        switch(a_msg->type)
        {
            case SKSE::MessagingInterface::kPostLoadGame:
                _OnGameLoad();
                break;
            case SKSE::MessagingInterface::kPostPostLoad:
                _OnPostPostLoad();
                break;
        }
    }
}