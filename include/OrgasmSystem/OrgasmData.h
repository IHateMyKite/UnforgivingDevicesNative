#pragma once
#include <map>

namespace ORS
{
    enum OrgasmVariable : uint8_t
    {
        vOrgasmRate              = 1,
        vOrgasmRateMult          = 2,
        vOrgasmResistence        = 3,
        vOrgasmResistenceMult    = 4,
        vOrgasmCapacity          = 5,
        vOrgasmForcing           = 6
    };

    enum OrgasmUpdateType : uint8_t
    {
        mSet                     = 0x1,
        mAdd                     = 0x2,
        mMultiply                = 0x3,
    };

    enum OrgasmMod : uint8_t
    {
        mNone                    = 0x0,
        mEdgeOnly                = 0x1
    };

    class OrgasmChangeData
    {
    public:
        char        Key[32]                 = "";
        float       OrgasmRate              = 0.0f;
        float       OrgasmRateMult          = 1.0f;
        float       OrgasmForcing           = 0.0f;
        float       OrgasmCapacity          = 100.0f;
        float       OrgasmResistence        = 3.5f;
        float       OrgasmResistenceMult    = 1.0;
        uint8_t     Mod                     = 0x00;
        uint32_t    EroZones                = 0x00000000; //up to 32 ero zones. Should be more than enought
    };

    enum EroZone : uint32_t
    {
        eNone                   = 0x00000000,
        eVagina1                = 0x00000001,
        eVagina2                = 0x00000002,
        eClitoris               = 0x00000004,
        ePenis1                 = 0x00000008,
        ePenis2                 = 0x00000010,
        ePenis3                 = 0x00000020,
        eNipples                = 0x00000040,
        eAnal1                  = 0x00000080,
        eAnal2                  = 0x00000100
    };

    class OrgasmEroZone
    {
    public:
        char        Alias[6];
        char        DispleyName[12];
        EroZone     EroZoneSlot = eNone;
        float       Multiplier  = 1.0f;
    };

    class OrgasmActorData
    {
    public:
        void    Update(float a_delta);
        float   GetOrgasmProgress() const;

        bool    OrgasmChangeExist(std::string a_key) const;
        bool    AddOrgasmChange(std::string a_key,  OrgasmMod a_mod,
                                                    EroZone a_erozones,
                                                    float a_orgasmRate, 
                                                    float a_orgasmRateMul, 
                                                    float a_orgasmForcing, 
                                                    float a_orgasmCapacity, 
                                                    float a_orgasmResistence, 
                                                    float a_orgasmResistenceMult
                                                    );
        bool    RemoveOrgasmChange(std::string a_key);
        bool    UpdateOrgasmChangeVar(std::string a_key, OrgasmVariable a_variable, float a_value, OrgasmUpdateType a_mod);
        float   GetOrgasmChangeVar(std::string a_key, OrgasmVariable a_variable);
        RE::Actor*  GetActor();
        void        SetActor(RE::Actor* a_actor);

        void    OnGameLoaded(SKSE::SerializationInterface*);
        void    OnGameSaved(SKSE::SerializationInterface*);
        void    OnRevert(SKSE::SerializationInterface*);
    private:
        inline float CalculateOrgasmProgress();
        inline float CalculateOrgasmRate();
        inline float CalculateOrgasmRateMult();
        inline float CalculateOrgasmForcing();
        inline float CalculateOrgasmCapacity();
        inline float CalculateOrgasmResistence();
        inline float CalculateOrgasmResistenceMult();
    private:
        RE::Actor*  _actor;
        std::map<std::string,OrgasmChangeData>  _Sources;
        OrgasmEroZone                           _EroZones[32] = 
        {
            {"VAGN1","Vagina 1" ,eVagina1},
            {"VAGN2","Vagina 2" ,eVagina2},
            {"CLITO","Clitoris" ,eClitoris},
            {"PENS1","Penis 1"  ,ePenis1},
            {"PENS2","Penis 2"  ,ePenis2},
            {"PENS3","Penis 3"  ,ePenis3},
            {"NIPPL","Nipples"  ,eNipples},
            {"ANAL1","Anal 1"   ,eAnal1},
            {"ANAL2","Anal 2"   ,eAnal2}
        };
        float       _OrgasmRate             = 0.0f;
        float       _AntiOrgasmRate         = 0.0f;
        float       _OrgasmRateMult         = 1.0f;

        float       _OrgasmForcing          = 0.0f;

        float       _OrgasmCapacityDef      = 100.0f;
        float       _OrgasmCapacity         = 100.0f;

        float       _OrgasmResistenceDef    = 3.5f;
        float       _OrgasmResistence       = 3.5f;
        float       _OrgasmResistenceMult   = 1.0;

        float       _OrgasmProgress         = 0.0f;

        int8_t      _Arousal                = 0;
    private:

    };

   extern RE::TESFaction* g_ArousalFaction;
}