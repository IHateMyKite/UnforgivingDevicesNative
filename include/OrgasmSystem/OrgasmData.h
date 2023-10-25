#pragma once
#include <map>

#include <boost/algorithm/clamp.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/ini_parser.hpp>

using boost::algorithm::clamp;

namespace ORS
{
    // === Calibration ===

    //how often will be expression updated (if set)
    #define EXPRUPDATETIME  2.0f

    //thresholds for updating expression
    #define EXPUPDATEMINTH  0.10f
    #define EXPUPDATEMAXTH  0.20f

    //orgasm progress required for linked widget to show
    #define WIDGETSHOWTH    0.025f

    //distance to travel in 1s for 100% orgasm rate
    #define BASEDISTANCE    2500.0f


    #define PDATAVERSION    1U

    // === ENums

    enum OrgasmVariable : uint8_t
    {
        vNone                       =  0,

        vOrgasmRate                 =  1,
        vOrgasmRateMult             =  2,
        vOrgasmResistence           =  3,
        vOrgasmResistenceMult       =  4,
        vOrgasmCapacity             =  5,
        vOrgasmForcing              =  6,

        vElapsedDuration            =  7,

        vArousal                    =  8,
        vArousalRate                =  9,
        vArousalRateMult            = 10,

        vEdgeDuration               = 11,
        vEdgeRemDuration            = 12,
        vEdgeThreshold              = 13,

        vHornyLevel                 = 14,

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
        mEdgeRandom                 = 0x02, //currently works same as mEdgeOnly
        //timer setting
        mTimed                      = 0x04, //orgasm change will be removed once time elapses. Duration is saved in last 16 bites of OrgasmMod passed to AddOrgasmChange function
        mTimeMod_Lin                = 0x08, //orgasm rate will decrease over time lineary

        mTimeMod_Exp                = 0x10, //orgasm rate will decrease over time exponencialy. Use this in combination with mTimed to make multiple timed changes
        //other
        mMakeKey                    = 0x20, //create new key if passed key is already used

        mArousingMovement           = 0x40  //scale orgasm and arousal rate based on traveled distance
        //7-15  = reserved
        //16-31 = Duration (seconds)
    };

    enum MeterWidgetType : uint8_t
    {
        tSkyUi                        = 0,
        tIWW                          = 1
    };

    enum ExpressionUpdateType : uint8_t
    {
        eSet    = 0,
        eReset  = 1
    };

    enum LinkedWidgetUpdateType : uint8_t
    {
        wShow   = 0,
        wHide   = 1
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
        eDefault                = 0x00000200    //when you dont care about ero zones
    };
    class OrgasmEroZone
    {
    public:
        char        Alias[6];
        char        DispleyName[12];
        EroZone     EroZoneSlot = eNone;
        float       Multiplier  = 1.0f;
    };

    // === Classes

    class OrgasmChangeData
    {
    public:
        char        Key[64]                 = "";
        float       OrgasmRateOriginal      = 0.0f;
        float       OrgasmRate              = 0.0f;
        float       OrgasmRateMult          = 0.0f;
        float       OrgasmForcing           = 0.0f;
        float       OrgasmCapacity          = 0.0f;
        float       OrgasmResistence        = 0.0f;
        float       OrgasmResistenceMult    = 0.0f;
        uint16_t    Duration                = -1;
        float       ElapsedDuration         = 0.0f;
        uint8_t     Mod                     = 0x00;
        uint32_t    EroZones                = 0x00000000; //up to 32 ero zones. Should be more than enought
        float       EdgeDuration            = 0.0f;
        float       EdgeRemDuration         = 0.0f;
        float       EdgeThreshold           = 0.9f;

        //arousal
        float       ArousalRate             = 0.0f;
        float       ArousalRateMult         = 0.0f;

        uint8_t     _reserved[16];
    };

    class OrgasmActorData
    {
    public:
        //OrgasmActorData(RE::Actor* a_actor) : _actor(a_actor){};
        void    Update(const float& a_delta);
        float   GetOrgasmProgress(int a_mod) const;
        float   GetOrgasmProgressLink() const { return clamp(100.0f*_PDATA.OrgasmProgress/_RDATA.OrgasmCapacity,0.0f,100.0f);}
        void    ResetOrgasmProgress();
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
        float   GetAntiOrgasmRate(){return _RDATA.AntiOrgasmRate;}

        void    LinkActorToMeter(std::string a_path,MeterWidgetType a_type,int a_id);
        void    UnlinkActorFromMeter();

        std::string MakeUniqueKey(std::string a_base);

        std::vector<std::string> GetAllOrgasmChanges();
        int     RemoveAllOrgasmChanges();

        void    Orgasm(void);

        RE::Actor*  GetActor();
        void        SetActor(RE::Actor* a_actor);

        //serde
        void    OnGameLoaded(SKSE::SerializationInterface*);
        void    OnGameSaved(SKSE::SerializationInterface*);
        void    OnRevert(SKSE::SerializationInterface*);

        void    UpdatePosition();
    private:
        inline float CalculateOrgasmProgress(const float& a_delta);
        inline float CalculateOrgasmRate(const float& a_delta);
        inline float CalculateOrgasmRateMult();
        inline float CalculateOrgasmForcing();
        inline float CalculateOrgasmCapacity();
        inline float CalculateOrgasmResistence();
        inline float CalculateOrgasmResistenceMult();
        inline float CalculateArousalRate(const float& a_delta);
        inline float CalculateArousalRateMult();
        inline float CalculateHornyLevel(const float& a_delta);
        inline void  ElapseChanges(const float& a_delta);
        inline void  UpdateWidget();

        inline void  UpdateExpression(const float& a_delta);

        inline void SendOrgasmEvent();
        inline void SendOrgasmExpressionEvent(ExpressionUpdateType a_type);
        inline void SendLinkedMeterEvent(LinkedWidgetUpdateType a_type);
    private:
        RE::Actor*  _actor;
        std::map<std::string,OrgasmChangeData>  _Sources;

        struct PERSIST_DATA_HEADER
        {
            
            uint8_t         version = PDATAVERSION; //always change when data structure is changed !!!
            uint16_t        size    = sizeof(PERSIST_DATA);
            uint16_t        ocsize  = sizeof(OrgasmChangeData);
        };

        PERSIST_DATA_HEADER _PDATAH;

        struct PERSIST_DATA
        {
            OrgasmEroZone   EroZones[32] = 
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
                {"DEFAU","Default"  ,eDefault},
            };
            float   OrgasmProgress         = 0.0f;
            float   HornyLevel             = 100.0f;
            uint8_t Reserved[32]; //reserved 32 bytes for future additiones
        };

        PERSIST_DATA _PDATA;

        struct RUNTIME_DATA
        {
            float   OrgasmRate             = 0.0f;
            float   AntiOrgasmRate         = 0.0f;
            float   OrgasmRateMult         = 1.0f;
            float   OrgasmRateTotal        = 0.0f;
            float   OrgasmRatePersist      = 0.0f;

            float   OrgasmForcing          = 0.0f;

            float   OrgasmCapacity         = 100.0f;

            float   OrgasmResistence       = 3.5f;
            float   OrgasmResistenceMult   = 1.0;

            float   OrgasmTimeout          = 0.0f;

            float   Arousal                = 0.0f;
            float   ArousalRate            = 0.0f;
            float   ArousalRatePersist     = 0.0f;
            float   ArousalRateMult        = 1.0f;

            bool                LinkedWidgetUsed       = false;
            std::string         LinkedWidgetPath       = "";
            MeterWidgetType     LinkedWidgetType       = tSkyUi;
            int32_t             LinkedWidgetId         = 0;
            bool                LinkedWidgetShown      = false;

            float               ExpressionTimer    = EXPRUPDATETIME;
            bool                ExpressionSet      = false;

            RE::NiPoint3 lastpos;
        };

        RUNTIME_DATA _RDATA;
    };
}