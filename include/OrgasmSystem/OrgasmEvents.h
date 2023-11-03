#pragma once

namespace ORS
{
    #define ORSREGISTEREVENT(a_name,a_arg)                          \
    {                                                               \
        if (a_alias == nullptr) return;                             \
        ORS::OrgasmEvents::GetSingleton()->a_name.Register(a_arg);  \
    }

    class OrgasmEvents
    {
    SINGLETONHEADER(OrgasmEvents)
    public:
        void RegisterPapyrus(RE::BSScript::IVirtualMachine *vm);
    public:
        SKSE::RegistrationSet<RE::Actor*,float,float,float,int> OrgasmEvent{"ORSEvent_OnActorOrgasm"};
        SKSE::RegistrationSet<RE::Actor*,int,float,float,float> ExpressionUpdateEvent{"ORSEvent_OnExpressionUpdate"};
    };

    inline void RegisterForOrgasmEvent_Ref(PAPYRUSFUNCHANDLE, RE::BGSRefAlias* a_alias)
    {
        ORSREGISTEREVENT(OrgasmEvent,a_alias)
    }

    inline void RegisterForOrgasmEvent_Form(PAPYRUSFUNCHANDLE, const RE::TESForm* a_alias)
    {
        ORSREGISTEREVENT(OrgasmEvent,a_alias)
    }

    inline void RegisterForExpressionUpdate_Ref(PAPYRUSFUNCHANDLE, const RE::BGSRefAlias* a_alias)
    {
        ORSREGISTEREVENT(ExpressionUpdateEvent,a_alias)
    }

    inline void RegisterForExpressionUpdate_Form(PAPYRUSFUNCHANDLE, const RE::TESForm* a_alias)
    {
        ORSREGISTEREVENT(ExpressionUpdateEvent,a_alias)
    }
}