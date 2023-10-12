#pragma once
#include <map>

#include <boost/algorithm/clamp.hpp>
using boost::algorithm::clamp;

namespace ORS
{

    #define EDGEDURATION 2.0f
    #define EDGETHRESHOLD 0.9f

    enum OrgasmVariable : uint8_t
    {
        vNone                       =  0,
        vOrgasmRate                 =  1,
        vOrgasmRateMult             =  2,
        vOrgasmResistence           =  3,
        vOrgasmResistenceMult       =  4,
        vOrgasmCapacity             =  5,
        vOrgasmForcing              =  6,
        vElapsedTime                =  7,
        vArousal                    =  8,
        vArousalRate                =  9,
        vArousalRateMult            = 10,

        vLast
    };

    enum OrgasmUpdateType : uint8_t
    {
        mSet                        = 1,
        mAdd                        = 2,
        mMultiply                   = 3,
    };

    enum OrgasmMod : uint32_t
    {
        //default modes
        mNone                       = 0x00,

        mEdgeOnly                   = 0x01,
        mEdgeRandom                 = 0x02,
        //timer setting
        mTimed                      = 0x04, //orgasm change will be removed once time elapses. Duration is saved in last 16 bites of OrgasmMod passed to AddOrgasmChange function
        mTimeMod_Lin                = 0x08, //orgasm rate will decrease over time lineary

        mTimeMod_Exp                = 0x10, //orgasm rate will decrease over time exponencialy. Use this in combination with mTimed to make multiple timed changes
        //other
        mMakeKey                    = 0x20  //create new key if passed key is already used
        //7-15  = reserved
        //16-31 = Duration (seconds)
    };

    enum MeterWidgetType : uint8_t
    {
        tSkyUi                        = 0,
        tIWW                          = 1
    };

    class OrgasmChangeData
    {
    public:
        char        Key[32]                 = "";
        float       OrgasmRateOriginal      = 0.0f;
        float       OrgasmRate              = 0.0f;
        float       OrgasmRateMult          = 1.0f;
        float       OrgasmForcing           = 0.0f;
        float       OrgasmCapacity          = 100.0f;
        float       OrgasmResistence        = 3.5f;
        float       OrgasmResistenceMult    = 1.0f;
        uint16_t    Duration                = -1;
        float       ElapsedDuration         = 0.0f;
        uint8_t     Mod                     = 0x00;
        uint32_t    EroZones                = 0x00000000; //up to 32 ero zones. Should be more than enought
        float       EdgeDuration            = 0.0f;
        float       EdgeElapsedDuration     = 0.0f;

        //arousal
        float       ArousalRate             = 0.0f;
        float       ArousalRateMult         = 1.0f;

        uint8_t     _reserved[16];
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
        eAnal2                  = 0x00000100,
        eSpecial                = 0x00000200    //only for orgasm resist minigame
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
        void    Update(const float& a_delta);
        float   GetOrgasmProgress(int a_mod) const;
        float   GetOrgasmProgressLink() const { return clamp(100.0f*_OrgasmProgress/_OrgasmCapacity,0.0f,100.0f);}
        void    ResetOrgasmProgress() { _OrgasmProgress = 0.0f; }
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
        bool    HaveOrgasmChange(std::string a_key);

        float   GetOrgasmVariable(OrgasmVariable a_variable);
        float   GetAntiOrgasmRate(){return _AntiOrgasmRate;}

        void    LinkActorToMeter(std::string a_path,MeterWidgetType a_type,int a_id);
        void    UnlinkActorFromMeter();

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
        inline float CalculateArousalRate();
        inline float CalculateArousalRateMult();
        inline void  ElapseChanges(const float& a_delta);
        inline void  UpdateWidget();
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
            {"ANAL2","Anal 2"   ,eAnal2},
            {"SPECL","Special"  ,eSpecial},
        };
        float   _OrgasmRate             = 0.0f;
        float   _AntiOrgasmRate         = 0.0f;
        float   _OrgasmRateMult         = 1.0f;
        float   _OrgasmRateTotal        = 0.0f;

        float   _OrgasmForcing          = 0.0f;

        float   _OrgasmCapacity         = 100.0f;

        float   _OrgasmResistence       = 3.5f;
        float   _OrgasmResistenceMult   = 1.0;

        float   _OrgasmProgress         = 0.0f;


        float   _ArousalReal            = 0.0f;
        float   _ArousalRate            = 0.0f;
        float   _ArousalRateMult        = 1.0f;

        int8_t  _Arousal                = 0;

        std::string         _LinkedWidgetPath       = "";
        MeterWidgetType     _LinkedWidgetType       = tSkyUi;
        int32_t             _LinkedWidgetId         = 0;
    private:

    };

   extern RE::TESFaction* g_ArousalFaction;
}