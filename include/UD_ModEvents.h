#pragma once

namespace UD
{
    class ModEvents
    {
    SINGLETONHEADER(ModEvents)
    public:
        SKSE::RegistrationSet<> HMTweenMenuEvent{"UDEvent_OnHMTweenMenuOpen"};
    };


    inline void RegisterForHMTweenMenu(PAPYRUSFUNCHANDLE, RE::BGSRefAlias* a_alias)
    {
        ModEvents::GetSingleton()->HMTweenMenuEvent.Register(a_alias);
    }

}