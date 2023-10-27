#pragma once

namespace UD
{
    class ModEvents
    {
    SINGLETONHEADER(ModEvents)
    public:
        SKSE::RegistrationSet<> HMTweenMenuEvent{"UDEvent_OnHMTweenMenuOpen"};
        SKSE::RegistrationSet<RE::Actor*,float,float> OrgasmEvent{"UDEvent_OnActorOrgasm"};
    };


    inline void RegisterForHMTweenMenu(PAPYRUSFUNCHANDLE, RE::BGSRefAlias* a_alias)
    {
        ModEvents::GetSingleton()->HMTweenMenuEvent.Register(a_alias);
    }

    inline void RegisterForOrgasmEvent_Ref(PAPYRUSFUNCHANDLE, RE::BGSRefAlias* a_alias)
    {
        if (a_alias == nullptr) return;
        ModEvents::GetSingleton()->OrgasmEvent.Register(a_alias);
    }

    inline void RegisterForOrgasmEvent_Form(PAPYRUSFUNCHANDLE, const RE::BGSBaseAlias* a_alias)
    {
        if (a_alias == nullptr) return;
        ModEvents::GetSingleton()->OrgasmEvent.Register(a_alias);
    }

}