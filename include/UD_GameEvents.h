#pragma once

namespace UD
{
    inline void _OnPostPostLoad();
    inline void _OnGameLoad();
    void OnMessageReceived(SKSE::MessagingInterface::Message* a_msg);
}