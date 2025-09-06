#pragma once
#include <OrgasmSystem/OrgasmData.h>
#include <OrgasmSystem/OrgasmEvents.h>
#include <UD_Spinlock.h>

namespace ORS
{
    class OrgasmActorData;

    typedef float(* ModifyArousal)(RE::Actor* actorRef, float value, bool sendevent);

    extern ModifyArousal OSLAModifyArousal;

    #define GETORGCHANGEANDVALIDATE(var,arg)        \
    OrgasmActorData& var = _actors[arg->GetHandle().native_handle()];            \
    var.SetActor(arg);

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
        bool    HaveOrgasmChange(RE::Actor* a_actor, std::string a_key);

        float   GetOrgasmProgress(RE::Actor* a_actor, int a_mod);
        void    ResetOrgasmProgress(RE::Actor* a_actor);
        float   GetOrgasmVariable(RE::Actor* a_actor, OrgasmVariable a_variable);
        float   GetAntiOrgasmRate(RE::Actor* a_actor);

        void    LinkActorToMeter(RE::Actor* a_actor,std::string a_path, MeterWidgetType a_type, int a_id);
        void    UnlinkActorFromMeter(RE::Actor* a_actor);
        std::string MakeUniqueKey(RE::Actor* a_actor,std::string a_base);
        std::vector<std::string> GetAllOrgasmChanges(RE::Actor* a_actor);
        int     RemoveAllOrgasmChanges(RE::Actor* a_actor);
        bool    IsOrgasming(RE::Actor* a_actor);
        int     GetOrgasmingCount(RE::Actor* a_actor);
        void    Orgasm(RE::Actor* a_actor);
        std::string GetHornyStatus(RE::Actor* a_actor);
        int     GetOrgasmFlags(RE::Actor* a_actor);
        bool    SetOrgasmFlags(RE::Actor* a_actor, int a_flags);
        bool    UseArousalFallback(void) const;

        void    RegisterPapyrusFunctions(RE::BSScript::IVirtualMachine *vm);

        void    OnGameLoaded(SKSE::SerializationInterface*);
        void    OnGameSaved(SKSE::SerializationInterface*);
        void    OnRevert(SKSE::SerializationInterface*);
    private:
        bool              _installed = false;
        mutable Utils::Spinlock  _lock;
        std::unordered_map<uint32_t,OrgasmActorData> _actors;
        RE::TESFaction* _arousalfaction = nullptr;
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

    inline bool   HaveOrgasmChange(PAPYRUSFUNCHANDLE,RE::Actor* a_actor,std::string a_key)
    {
        return OrgasmManager::GetSingleton()->HaveOrgasmChange(a_actor,a_key);
    }

    inline float   GetOrgasmProgress(PAPYRUSFUNCHANDLE,RE::Actor* a_actor,int a_mod)
    {
        return OrgasmManager::GetSingleton()->GetOrgasmProgress(a_actor,a_mod);
    }

    inline void    ResetOrgasmProgress(PAPYRUSFUNCHANDLE,RE::Actor* a_actor)
    {
        OrgasmManager::GetSingleton()->ResetOrgasmProgress(a_actor);
    }

    inline float   GetOrgasmVariable(PAPYRUSFUNCHANDLE,RE::Actor* a_actor, int a_variable)
    {
        return OrgasmManager::GetSingleton()->GetOrgasmVariable(a_actor,(OrgasmVariable)a_variable);
    }

    inline float   GetAntiOrgasmRate(PAPYRUSFUNCHANDLE,RE::Actor* a_actor)
    {
        return OrgasmManager::GetSingleton()->GetAntiOrgasmRate(a_actor);
    }

    inline void   LinkActorToMeter(PAPYRUSFUNCHANDLE,RE::Actor* a_actor,std::string a_path, int a_type, int a_id)
    {
        OrgasmManager::GetSingleton()->LinkActorToMeter(a_actor,a_path, (MeterWidgetType)a_type,a_id);
    }

    inline void   UnlinkActorFromMeter(PAPYRUSFUNCHANDLE,RE::Actor* a_actor)
    {
        OrgasmManager::GetSingleton()->UnlinkActorFromMeter(a_actor);
    }

    inline std::string MakeUniqueKey(PAPYRUSFUNCHANDLE,RE::Actor* a_actor,std::string a_base)
    {
        return OrgasmManager::GetSingleton()->MakeUniqueKey(a_actor,a_base);
    }

    inline std::vector<std::string> GetAllOrgasmChanges(PAPYRUSFUNCHANDLE,RE::Actor* a_actor)
    {
        return OrgasmManager::GetSingleton()->GetAllOrgasmChanges(a_actor);
    }

    inline int RemoveAllOrgasmChanges(PAPYRUSFUNCHANDLE,RE::Actor* a_actor)
    {
        return OrgasmManager::GetSingleton()->RemoveAllOrgasmChanges(a_actor);
    }

    inline bool IsOrgasming(PAPYRUSFUNCHANDLE,RE::Actor* a_actor)
    {
        return OrgasmManager::GetSingleton()->IsOrgasming(a_actor);
    }

    inline int GetOrgasmingCount(PAPYRUSFUNCHANDLE,RE::Actor* a_actor)
    {
        return OrgasmManager::GetSingleton()->GetOrgasmingCount(a_actor);
    }

    inline void ForceOrgasm(PAPYRUSFUNCHANDLE,RE::Actor* a_actor)
    {
        OrgasmManager::GetSingleton()->Orgasm(a_actor);
    }

    inline std::string GetHornyStatus(PAPYRUSFUNCHANDLE,RE::Actor* a_actor)
    {
        return OrgasmManager::GetSingleton()->GetHornyStatus(a_actor);
    }

    inline int GetOrgasmFlags(PAPYRUSFUNCHANDLE,RE::Actor* a_actor)
    {
        return OrgasmManager::GetSingleton()->GetOrgasmFlags(a_actor);
    }

    inline bool SetOrgasmFlags(PAPYRUSFUNCHANDLE,RE::Actor* a_actor, int a_flags)
    {
        return OrgasmManager::GetSingleton()->SetOrgasmFlags(a_actor,a_flags);
    }

    inline bool UseArousalFallback(PAPYRUSFUNCHANDLE)
    {
        return OrgasmManager::GetSingleton()->UseArousalFallback();
    }
}