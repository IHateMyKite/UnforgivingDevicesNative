#pragma once
#include <OrgasmSystem/OrgasmData.h>

namespace ORS
{
    //update time in ms
    #define ORSUPTIME 100

    class OrgasmActorData;

    class OrgasmManager
    {
    SINGLETONHEADER(OrgasmManager)
    public:
        void    Setup();
        void    Update(float a_delta);
        bool    AddOrgasmChange(RE::Actor* a_actor, 
                                std::string a_key,  
                                OrgasmMod a_mod,
                                EroZone a_erozones,
                                float a_orgasmRate, 
                                float a_orgasmRateMult          = 0.0f,
                                float a_orgasmForcing           = 0.0f,
                                float a_orgasmCapacity          = 0.0f,
                                float a_orgasmResisten          = 0.0f,
                                float a_orgasmResistenceMult    = 0.0f);
        bool    AddOrgasmChange(RE::Actor* a_actor, 
                                std::string a_key,  
                                OrgasmMod a_mod,
                                uint32_t a_erozones,
                                float a_orgasmRate, 
                                float a_orgasmRateMult          = 0.0f,
                                float a_orgasmForcing           = 0.0f,
                                float a_orgasmCapacity          = 0.0f,
                                float a_orgasmResisten          = 0.0f,
                                float a_orgasmResistenceMult    = 0.0f);
        bool    RemoveOrgasmChange(RE::Actor* a_actor, std::string a_key);
        bool    UpdateOrgasmChangeVar(RE::Actor* a_actor, std::string a_key, OrgasmVariable a_variable, float a_value, OrgasmUpdateType a_mod);
        float   GetOrgasmChangeVar(RE::Actor* a_actor, std::string a_key, OrgasmVariable a_variable);

        float   GetOrgasmProgress(RE::Actor* a_actor);
        void    RegisterPapyrusFunctions(RE::BSScript::IVirtualMachine *vm);

        void    OnGameLoaded(SKSE::SerializationInterface*);
        void    OnGameSaved(SKSE::SerializationInterface*);
        void    OnRevert(SKSE::SerializationInterface*);
    private:
        bool                _installed = false;
        mutable std::mutex  _lock;
        std::unordered_map<RE::Actor*,OrgasmActorData> _actors;
    };

    inline bool    AddOrgasmChange(PAPYRUSFUNCHANDLE,
                            RE::Actor* a_actor, 
                            std::string a_key,  
                            int     a_mod,
                            int     a_erozones,
                            float   a_orgasmRate, 
                            float   a_orgasmRateMult,
                            float   a_orgasmForcing,
                            float   a_orgasmCapacity,
                            float   a_orgasmResistence,
                            float   a_orgasmResistenceMult)
    {
        return OrgasmManager::GetSingleton()->AddOrgasmChange(a_actor,a_key,(OrgasmMod)a_mod,a_erozones,a_orgasmRate,a_orgasmRateMult,a_orgasmForcing,a_orgasmCapacity,a_orgasmResistence,a_orgasmResistenceMult);
    }

    inline bool    RemoveOrgasmChange(PAPYRUSFUNCHANDLE,RE::Actor* a_actor, std::string a_key)
    {
        return OrgasmManager::GetSingleton()->RemoveOrgasmChange(a_actor,a_key);
    }

    inline bool    UpdateOrgasmChangeVar(PAPYRUSFUNCHANDLE,RE::Actor* a_actor, std::string a_key, int a_variable, float a_value, int a_mod)
    {
        return OrgasmManager::GetSingleton()->UpdateOrgasmChangeVar(a_actor,a_key,(OrgasmVariable)a_variable,a_value,(OrgasmUpdateType)a_mod);
    }

    inline float   GetOrgasmChangeVar(PAPYRUSFUNCHANDLE,RE::Actor* a_actor, std::string a_key, int a_variable)
    {
        return OrgasmManager::GetSingleton()->GetOrgasmChangeVar(a_actor,a_key,(OrgasmVariable)a_variable);
    }

    inline float   GetOrgasmProgress(PAPYRUSFUNCHANDLE,RE::Actor* a_actor)
    {
        return OrgasmManager::GetSingleton()->GetOrgasmProgress(a_actor);
    }

}