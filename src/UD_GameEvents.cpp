#include <UD_GameEvents.h>
#include <UD_H.h>

namespace UD
{
    void _OnGameLoad()
    {
        SKSE::log::info("::_OnGameLoad called, effect started {}",ActorValueUpdateHook::started);
        //remove effect in case that user reloaded the game without exit
        if (ActorValueUpdateHook::started) ActorValueUpdateHook::RemoveAll();
    }

    void OnMessageReceived(SKSE::MessagingInterface::Message* a_msg)
    {
        switch(a_msg->type)
        {
            case SKSE::MessagingInterface::kPostLoadGame:
            _OnGameLoad();
            break;
        }
    }
}