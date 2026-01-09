#pragma once

namespace UD
{
    inline void _OnPostPostLoad();
    inline void _OnGameLoad(bool a_newGame);
    void OnMessageReceived(SKSE::MessagingInterface::Message* a_msg);
}