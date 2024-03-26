#include <OrgasmSystem/OrgasmData.h>
#include <OrgasmSystem/OrgasmManager.h>
#include <OrgasmSystem/OrgasmConfig.h>
#include <OrgasmSystem/OrgasmEvents.h>
#include <UD_UI.h>
#include <UD_ModEvents.h>

using boost::algorithm::clamp;

std::vector<ORS::HornyLevel> ORS::g_HornyLevels = {
    {0.0f,20.0f,"You are feeling very exhausted!","Very exhausted"},
    {30.0f,75.0f,"You are feeling exhausted","Exhausted"},
    {75.0f,125.0f,"Your libido is under control","Normal"},
    {125.0f,250.0f,"You are feeling horny","Horny"},
    {250.0f,350.0f,"You are feeling increadibly horny","Increadibly horny"},
    {350.0f,400.0f,"You want to cum badly","Wants to cum badly"}
};

void ORS::OrgasmActorData::Update(const float& a_delta)
{
    //UniqueLock updatelock(lock); //apply spinlock

    if (_RDATA.Actor == nullptr) return;

    if (_RDATA.OrgasmTimer > 0.0f)
    {
        _RDATA.OrgasmTimer -= a_delta;
    }
    else
    {
        if (_RDATA.OrgasmCount > 0)
        {
            SendOrgasmExpressionEvent(eOrgasmReset);
            _RDATA.ExpressionTimer = 0.0f;
        }
        _RDATA.OrgasmTimer = 0.0f;
        _RDATA.OrgasmCount = 0;
    }

    _RDATA.ArousalRate            = CalculateArousalRate(a_delta);
    _RDATA.ArousalRateMult        = CalculateArousalRateMult();

    const float loc_da            = _RDATA.ArousalRate*_RDATA.ArousalRateMult*a_delta;

    _RDATA.ArousalEventTimer -= a_delta;

    bool loc_sendarousalevent = false;
    if (_RDATA.ArousalEventTimer <= 0.0f && (_RDATA.ArousalEventLastValue != _RDATA.Arousal))
    {
        static Config*      loc_config      = Config::GetSingleton();
        static const float  loc_updnpc      = loc_config->GetVariable<float>("Arousal.fArousalEventTimeNPC",5.0f);
        static const float  loc_updplayer   = loc_config->GetVariable<float>("Arousal.fArousalEventTimePlayer",1.0f);
        if (_RDATA.Actor->IsPlayerRef()) _RDATA.ArousalEventTimer = loc_updplayer;
        else _RDATA.ArousalEventTimer = loc_updnpc;

        if (_RDATA.Actor->Is3DLoaded()) loc_sendarousalevent = true;
        _RDATA.ArousalEventLastValue = _RDATA.Arousal;
    }

    if (OSLAModifyArousal != nullptr && loc_da != 0.0f) _RDATA.Arousal = OSLAModifyArousal(_RDATA.Actor,loc_da,loc_sendarousalevent);

    _RDATA.OrgasmRate             = CalculateOrgasmRate(a_delta);
    _RDATA.OrgasmRateMult         = CalculateOrgasmRateMult();
    _RDATA.OrgasmForcing          = CalculateOrgasmForcing();
    _RDATA.OrgasmCapacity         = CalculateOrgasmCapacity();
    _RDATA.OrgasmResistence       = CalculateOrgasmResistence();
    _RDATA.OrgasmResistenceMult   = CalculateOrgasmResistenceMult();

    _PDATA.OrgasmProgress         = CalculateOrgasmProgress(a_delta);

    ElapseChanges(a_delta);

    _RDATA.OrgasmRatePersist  = _RDATA.OrgasmRate;
    _RDATA.ArousalRatePersist = _RDATA.ArousalRate;

    _PDATA.HornyLevel = CalculateHornyLevel(a_delta);

    UpdateWidget();
    UpdatePosition();
    UpdateExpression(a_delta);

    if (_PDATA.OrgasmProgress >= 100.0) 
    {
        Orgasm();
    }

    if (IsPlayer()) CheckHornyLevel();

    //LOG("ORS::OrgasmActorData::Update({}) - {} = {} - {} --- T= {}",_actor->GetName(),_OrgasmProgress,_OrgasmRate,_AntiOrgasmRate,_OrgasmRateTotal)
}

float ORS::OrgasmActorData::GetOrgasmProgress(int a_mod) const
{
    if (a_mod == 0) return _PDATA.OrgasmProgress;
    else return _RDATA.OrgasmCapacity > 0.0f ? _PDATA.OrgasmProgress/_RDATA.OrgasmCapacity : 0.0f;
}

float ORS::OrgasmActorData::GetOrgasmProgressUI() const
{ 
    //UniqueLock uiLock(lock);
    return clamp(100.0f*_PDATA.OrgasmProgress/_RDATA.OrgasmCapacity,0.0f,100.0f);
}

void ORS::OrgasmActorData::ResetOrgasmProgress()
{
    //LOG("OrgasmManager::ResetOrgasmProgress({})",_actor->GetName())
    _PDATA.OrgasmProgress = 0.0f;
}

bool ORS::OrgasmActorData::OrgasmChangeExist(std::string a_key) const
{
    return _Sources.find(a_key) != _Sources.end();
}

bool ORS::OrgasmActorData::AddOrgasmChange(std::string a_key,   OrgasmMod a_mod,
                                                                EroZone a_erozones,
                                                                float a_orgasmRate,
                                                                float a_orgasmRateMul, 
                                                                float a_orgasmForcing, 
                                                                float a_orgasmCapacity, 
                                                                float a_orgasmResistence, 
                                                                float a_orgasmResistenceMult)
{
    if (OrgasmChangeExist(a_key)) 
    {   
        if (a_mod & mMakeKey)
        {
            a_key = MakeUniqueKey(a_key);
        }
        else
        {
            return false;
        }
        
    }

    //LOG("OrgasmActorData::AddOrgasmChange({},{},{})",_actor->GetName(),a_key,a_orgasmRate)

    a_key.copy(_Sources[a_key].Key,a_key.size() + 1); //+ 1 so it also copy \0 character
    _Sources[a_key].OrgasmRateOriginal      = a_orgasmRate;
    _Sources[a_key].OrgasmRate              = a_orgasmRate;
    _Sources[a_key].OrgasmRateMult          = a_orgasmRateMul;
    _Sources[a_key].OrgasmForcing           = a_orgasmForcing;
    _Sources[a_key].OrgasmCapacity          = a_orgasmCapacity;
    _Sources[a_key].OrgasmResistence        = a_orgasmResistence;
    _Sources[a_key].OrgasmResistenceMult    = a_orgasmResistenceMult;

    //check if orgasm data is timed
    if (a_mod & mTimed)
    {
        _Sources[a_key].Duration = (0xFFFF0000 & a_mod) >> 16;

        //check if duration is set to 0. If yes, make effect last 3s
        if (_Sources[a_key].Duration == 0)
        {
            _Sources[a_key].Duration = 3;
        }

        _Sources[a_key].ElapsedDuration = 0.0f;
    }

    _Sources[a_key].Mod                     = static_cast<uint8_t>(a_mod & 0x000000FF);
    _Sources[a_key].EroZones                = a_erozones;

    return true;
}

bool ORS::OrgasmActorData::RemoveOrgasmChange(std::string a_key)
{
    //LOG("OrgasmActorData::RemoveOrgasmChange({},{})",_actor->GetName(),a_key)
    return _Sources.erase(a_key) > 0;
}

bool ORS::OrgasmActorData::UpdateOrgasmChangeVar(std::string a_key, OrgasmVariable a_variable, float a_value, OrgasmUpdateType a_mod)
{
    if (!OrgasmChangeExist(a_key))
    {
        WARN("OrgasmActorData::UpdateOrgasmChangeVar({},{},{},{}) cant find key!",a_key,a_variable,a_value,a_mod)
        return false;
    }

    float* loc_var = nullptr;

    //LOG("OrgasmActorData::UpdateOrgasmChangeVar({},{},{},{})",a_key,a_variable,a_value,a_mod)

    #define UPDORGVAR(var)                  \
        case v##var:                        \
            loc_var = &_Sources[a_key].var; \
            break;

    switch (a_variable)
    {
        UPDORGVAR(OrgasmRate)
        UPDORGVAR(OrgasmRateMult)
        UPDORGVAR(OrgasmResistence)
        UPDORGVAR(OrgasmResistenceMult)
        UPDORGVAR(OrgasmCapacity)
        UPDORGVAR(OrgasmForcing)
        UPDORGVAR(ElapsedDuration)
        UPDORGVAR(ArousalRate)
        UPDORGVAR(ArousalRateMult)
        UPDORGVAR(EdgeDuration)
        UPDORGVAR(EdgeRemDuration)
        UPDORGVAR(EdgeThreshold)
        default:
            return false;
    };

    #undef UPDORGVAR

    switch (a_mod)
    {
        case mSet:
            *loc_var = a_value;
            if (a_variable == vOrgasmRate) _Sources[a_key].OrgasmRateOriginal = a_value;
            //LOG("OrgasmActorData::UpdateOrgasmChangeVar({},{},{},{}) - Set to {}",a_key,a_variable,a_value,a_mod,*loc_var)
            return true;
        case mAdd:
            *loc_var += a_value;
            if (a_variable == vOrgasmRate) _Sources[a_key].OrgasmRateOriginal += a_value;
            //LOG("OrgasmActorData::UpdateOrgasmChangeVar({},{},{},{}) - Increased to {}",a_key,a_variable,a_value,a_mod,*loc_var)
            return true;
        case mMultiply: 
            *loc_var *= a_value;
            if (a_variable == vOrgasmRate) _Sources[a_key].OrgasmRateOriginal *= a_value;
            //LOG("OrgasmActorData::UpdateOrgasmChangeVar({},{},{},{}) - Multiplied to {}",a_key,a_variable,a_value,a_mod,*loc_var)
            return true;
        default:
            return false;
    }

    return false;
}

float ORS::OrgasmActorData::GetOrgasmChangeVar(std::string a_key, OrgasmVariable a_variable)
{
    if (!OrgasmChangeExist(a_key)) return 0.0f;
    

    #define GETORGVAR(var)                  \
        case v##var: return _Sources[a_key].var;

    switch (a_variable)
    {
        GETORGVAR(OrgasmRate)
        GETORGVAR(OrgasmRateMult)
        GETORGVAR(OrgasmResistence)
        GETORGVAR(OrgasmResistenceMult)
        GETORGVAR(OrgasmCapacity)
        GETORGVAR(OrgasmForcing)
        GETORGVAR(ElapsedDuration)
        GETORGVAR(ArousalRate)
        GETORGVAR(ArousalRateMult)
        GETORGVAR(EdgeDuration)
        GETORGVAR(EdgeRemDuration)
        GETORGVAR(EdgeThreshold)
        default:
            return 0.0f;
    };
}

bool ORS::OrgasmActorData::HaveOrgasmChange(std::string a_key)
{
    return (_Sources.find(a_key) != _Sources.end());
}

float ORS::OrgasmActorData::GetOrgasmVariable(OrgasmVariable a_variable)
{
    switch (a_variable)
    {
        case vOrgasmRate:       return _RDATA.OrgasmRatePersist;
        case vOrgasmRateMult:   return CalculateOrgasmRateMult();
        case vOrgasmResistence: return CalculateOrgasmResistence();
        case vOrgasmResistenceMult: return CalculateOrgasmResistenceMult();
        case vOrgasmCapacity:   return CalculateOrgasmCapacity();
        case vOrgasmForcing:    return CalculateOrgasmForcing();
        case vArousal:          return _RDATA.Arousal;
        case vArousalRate:      return _RDATA.ArousalRatePersist;
        case vArousalRateMult:  return CalculateArousalRateMult();
        case vHornyLevel:       return _PDATA.HornyLevel;
        default:                return 0.0f;
    };
}

void ORS::OrgasmActorData::LinkActorToMeter(std::string a_path, MeterWidgetType a_type, int a_id)
{
    _RDATA.LinkedWidgetPath   = a_path;
    _RDATA.LinkedWidgetType   = a_type;
    _RDATA.LinkedWidgetId     = a_id; 
    _RDATA.LinkedWidgetUsed   = true;

    switch (a_type)
    {
        case tSkyUi : UD::MeterManager::SetExtCalcSkyUi(_RDATA.LinkedWidgetPath,this,&OrgasmActorData::GetOrgasmProgressUI); break;
        case tIWW   : UD::MeterManager::SetExtCalcIWW(a_id,this,&OrgasmActorData::GetOrgasmProgressUI); break;
    }
}

std::string ORS::OrgasmActorData::GetHornyStatus()
{
    const float loc_hornylevel = _PDATA.HornyLevel;
    for (auto&& it : g_HornyLevels)
    {
        const bool loc_inrange = ((loc_hornylevel >= it.Min) && (loc_hornylevel <= it.Max));
        if (loc_inrange)
        {
            return it.Status;
        }
    }
    return "ERROR";
}

void ORS::OrgasmActorData::UnlinkActorFromMeter()
{
     _RDATA.LinkedWidgetUsed  = false;

    switch (_RDATA.LinkedWidgetType)
    {
        case tSkyUi : UD::MeterManager::UnsetExtCalcSkyUi(_RDATA.LinkedWidgetPath); break;
        case tIWW   : UD::MeterManager::UnsetExtCalcIWW(_RDATA.LinkedWidgetId); break;
    }

    _RDATA.LinkedWidgetPath   = "";
    _RDATA.LinkedWidgetType   = tSkyUi;
    _RDATA.LinkedWidgetId     = 0; 
}

std::string ORS::OrgasmActorData::MakeUniqueKey(std::string a_base)
{
    uint16_t loc_suf = 0;
    std::string loc_res = std::format("{}_{:03}",a_base,loc_suf);
    while (HaveOrgasmChange(loc_res))
    {
        loc_suf++;
        loc_res = std::format("{}_{:03}",a_base,loc_suf);
    }
    return loc_res;
}

std::vector<std::string> ORS::OrgasmActorData::GetAllOrgasmChanges()
{
    std::vector<std::string> loc_res;
    for (auto&& it : _Sources) loc_res.push_back(it.first);
    return loc_res;
}

int ORS::OrgasmActorData::RemoveAllOrgasmChanges()
{
    int loc_res = static_cast<int>(_Sources.size());
    _Sources.clear();
    return loc_res;
}

void ORS::OrgasmActorData::Orgasm(void)
{
    static Config* loc_config = Config::GetSingleton();
    static const float loc_durmin  = loc_config->GetVariable<float>("Orgasm.fOrgasmDurationMin",15.0f);
    static const float loc_durmax  = loc_config->GetVariable<float>("Orgasm.fOrgasmDurationMax",40.0f);
    static const float loc_duradd  = loc_config->GetVariable<float>("Orgasm.fOrgasmDurationAdd",5.0f);
    static const int   loc_timeout = loc_config->GetVariable<int>("Orgasm.iOrgasmTimeout",10);

    if (_RDATA.OrgasmTimer == 0.0f) 
    {
        _RDATA.OrgasmTimer = std::lerp(loc_durmin,loc_durmax,clamp(_PDATA.HornyLevel - 100.0f,0.0f,300.0f)/300.0f);
    }
    else _RDATA.OrgasmTimer += loc_duradd;
    
    _RDATA.OrgasmCount++;

    std::string loc_key = MakeUniqueKey("PostOrgasm");

    AddOrgasmChange(loc_key,(OrgasmMod)(0x00004 | (loc_timeout << 16)),eDefault,-15.0f,-0.25f,0.0f,0.0f,0.25f,0.25f);
    UpdateOrgasmChangeVar(loc_key, vArousalRate, -15.0f, mSet);

    SendOrgasmEvent();
    SendOrgasmExpressionEvent(eOrgasmSet);
}

RE::Actor* ORS::OrgasmActorData::GetActor()
{
    return _RDATA.Actor;
}

void ORS::OrgasmActorData::SetActor(RE::Actor* a_actor)
{
    _RDATA.Actor = a_actor;
}

ORS::OrgasmActorData& ORS::OrgasmActorData::operator=(const ORS::OrgasmActorData& a_oad)
{
    if (this == &a_oad) return *this;

    _RDATA = a_oad._RDATA;
    _PDATA = a_oad._PDATA;
    _PDATAH = a_oad._PDATAH;
    _Sources = a_oad._Sources;

    return *this;
}

ORS::OrgasmActorData::OrgasmActorData(const ORS::OrgasmActorData& a_oad)
{
    _RDATA = a_oad._RDATA;
    _PDATA = a_oad._PDATA;
    _PDATAH = a_oad._PDATAH;
    _Sources = a_oad._Sources;
}

void ORS::OrgasmActorData::OnGameLoaded(SKSE::SerializationInterface* serde)
{
    static uint8_t loc_trashbuffer[2000];

    serde->ReadRecordData(&_PDATAH,sizeof(PERSIST_DATA_HEADER));

    if (_PDATAH.version == PDATAVERSION)
    {
        serde->ReadRecordData(&_PDATA,_PDATAH.size);

        //load number of sources
        uint8_t loc_ssize = 0;
        serde->ReadRecordData(&loc_ssize,sizeof(uint8_t));

        for (uint8_t i = 0; i < loc_ssize; i++)
        {
            //save size of key string
            uint8_t loc_keysize = 0; //+ 1 for null character
            serde->ReadRecordData(&loc_keysize,sizeof(uint8_t));

            char loc_buffer[64] = {};
            serde->ReadRecordData(loc_buffer,loc_keysize);

            std::string loc_key = loc_buffer;

            _Sources[loc_key] = OrgasmChangeData();
            serde->ReadRecordData(&_Sources[loc_key],sizeof(OrgasmChangeData));
        }
    }
    else
    {
        WARN("Not loading cosave data because data version is old!")
        serde->ReadRecordData(loc_trashbuffer,_PDATAH.size);

        uint8_t loc_ocnum = 0;
        serde->ReadRecordData(&loc_ocnum,sizeof(uint8_t));

        serde->ReadRecordData(loc_trashbuffer,_PDATAH.ocsize*loc_ocnum);

        //TODO - version resolve for future versions
    }

    //update to set up local variables
    Update(0.0f);
}

void ORS::OrgasmActorData::OnGameSaved(SKSE::SerializationInterface* serde)
{
    serde->WriteRecordData(&_PDATAH,sizeof(PERSIST_DATA_HEADER));
    serde->WriteRecordData(&_PDATA,sizeof(PERSIST_DATA));

    //save number of sources
    const uint8_t loc_ssize = static_cast<uint8_t>(_Sources.size());
    serde->WriteRecordData(&loc_ssize,sizeof(uint8_t));

    for (auto&& it : _Sources)
    {
        //save size of key string
        const uint8_t loc_keysize = static_cast<uint8_t>(it.first.size() + 1); // + 1 for null character
        serde->WriteRecordData(&loc_keysize,sizeof(uint8_t));
        serde->WriteRecordData(it.first.data(),loc_keysize);
        serde->WriteRecordData(&it.second,sizeof(OrgasmChangeData));
    }
}

void ORS::OrgasmActorData::OnRevert(SKSE::SerializationInterface* serde)
{
    _Sources.clear();
}

float ORS::OrgasmActorData::CalculateOrgasmProgress(const float& a_delta)
{
    float loc_res = _PDATA.OrgasmProgress;

    loc_res += (_RDATA.OrgasmRate*_RDATA.OrgasmRateMult*a_delta*(clamp(_PDATA.HornyLevel/200.0f,0.25f,1.25f)));

    const float loc_ArousalMult   = clamp(std::pow(10.0f,clamp(100.0f/clamp(_RDATA.Arousal,1.0f,100.0f),1.0f,2.0f) - 1.0f),1.0f,100.0f);;
    _RDATA.AntiOrgasmRate         = static_cast<float>(loc_ArousalMult*_RDATA.OrgasmResistenceMult*_RDATA.OrgasmResistence);

    const float loc_OrgasmRateAfterMult = _RDATA.OrgasmRate > 0.0f ? _RDATA.OrgasmRate*_RDATA.OrgasmRateMult : _RDATA.OrgasmRate;

    if (loc_OrgasmRateAfterMult > 0.0f)
    {
        loc_res -= static_cast<float>(_RDATA.AntiOrgasmRate*loc_res*a_delta/100.0f);
        _RDATA.OrgasmRateTotal = loc_OrgasmRateAfterMult - _RDATA.AntiOrgasmRate;
    }
    else
    {
        loc_res -= static_cast<float>(4*_RDATA.AntiOrgasmRate*loc_res*a_delta/100.0f);
        if (loc_res/_RDATA.OrgasmCapacity < 0.1) loc_res -= 0.1f; //help orgasm progress reach 0 when its low
        _RDATA.OrgasmRateTotal = loc_OrgasmRateAfterMult - 4*_RDATA.AntiOrgasmRate;
    }

    loc_res = clamp(loc_res,0.0f,_RDATA.OrgasmCapacity);
    if (loc_res < 0.0f) loc_res = 0.0f;

    return loc_res;
}

float ORS::OrgasmActorData::CalculateOrgasmRate(const float& a_delta)
{
    static Config* loc_config = Config::GetSingleton();
    float loc_res = 0.0f;

    static const float loc_arousalths = ORS::Config::GetSingleton()->GetVariable<float>("Arousal.fOrgasmThreshold",35.0f);
    const bool loc_canincrease = (_RDATA.Arousal >= loc_arousalths);

    for (auto&& it1 : _Sources) 
    {   
        OrgasmChangeData& loc_ocd = it1.second;
        const float& loc_or = loc_ocd.OrgasmRate;
        if (loc_or == 0.0f) continue;

        //check if actor doesnt have enought arousal for orgasm rate to increase
        if (!loc_canincrease && loc_or > 0.0f) continue;

        float           loc_mult        = 0.0f;
        const uint32_t& loc_erozones    = loc_ocd.EroZones;

        for (auto && it2 : _PDATA.EroZones)
        {
            //use smalles value
            if ((it2.EroZoneSlot & loc_erozones) && (it2.Multiplier < loc_mult || loc_mult == 0.0f) && (it2.Multiplier > 0.0f)) 
            {
                loc_mult = it2.Multiplier;
            }
        }

        if ((loc_ocd.Mod & mArousingMovement) && (a_delta > 0.0f))
        {
            {
                const auto loc_currentpos = _RDATA.Actor->GetPosition();
                const float loc_distance = loc_currentpos.GetSquaredDistance(_RDATA.lastpos);
                static const float loc_basedistance = loc_config->GetVariable<float>("Orgasm.fBaseDistance",2500.0f)*a_delta;

                if (loc_basedistance > 0.0f) loc_mult *= clamp(loc_distance/loc_basedistance,0.0f,10.0f);

                //LOG("OrgasmActorData::CalculateOrgasmRate() - Distance= {}, Base distance = {}, Distance multiplier = {}",loc_distance,loc_basedistance,loc_distance/loc_basedistance)
            }
        }

        //only if no edging is active
        if ((!(loc_ocd.Mod & (mEdgeOnly | mEdgeRandom)) || (GetOrgasmProgress(1) < loc_ocd.EdgeThreshold)) && loc_ocd.EdgeRemDuration <= 0.0f) 
        {
            //LOG("OrgasmActorData::CalculateOrgasmRate({},{}) - Changing orgasm rate by {} - Mult = {}",_actor->GetName(),it1.first,it1.second.OrgasmRate*loc_mult,loc_mult)
            loc_res += (loc_or > 0.0f ? loc_or*loc_mult : loc_or);
        }
        else if (loc_ocd.EdgeRemDuration <= 0.0f) 
        {
            loc_ocd.EdgeRemDuration = loc_ocd.EdgeDuration; //set edge timeout
        }
    }

    loc_res = clamp(loc_res,-5000.0f,5000.0f);
    //LOG("OrgasmActorData::CalculateOrgasmRate({}) - Final orgasm rate = {} ",_actor->GetName(),loc_res)

    return loc_res;
}

float ORS::OrgasmActorData::CalculateOrgasmRateMult()
{
    float loc_res = 1.0f;
    for (auto&& it : _Sources) loc_res += it.second.OrgasmRateMult;
    return clamp(loc_res,0.0f,100.0f);
}

float ORS::OrgasmActorData::CalculateOrgasmForcing()
{
    float loc_res = 0.0f;
    for (auto&& it : _Sources) loc_res += it.second.OrgasmForcing;
    return clamp(loc_res,0.0f,10000.0f);
}

float ORS::OrgasmActorData::CalculateOrgasmCapacity()
{
    float loc_res = 100.0f;
    for (auto&& it : _Sources) loc_res += it.second.OrgasmCapacity;
    return clamp(loc_res,1.0f,10000.0f);
}

float ORS::OrgasmActorData::CalculateOrgasmResistence()
{
    float loc_res = 3.5f;
    for (auto&& it : _Sources) loc_res += it.second.OrgasmResistence;
    return clamp(loc_res,0.0f,100.0f);
}

float ORS::OrgasmActorData::CalculateOrgasmResistenceMult()
{
    float loc_res = 1.0f;
    for (auto&& it : _Sources) loc_res += it.second.OrgasmResistenceMult;
    return clamp(loc_res,0.0f,100.0f);
}

inline float ORS::OrgasmActorData::CalculateArousalRate(const float& a_delta)
{
    static Config* loc_config = Config::GetSingleton();
    float loc_res = 0.0f;
    for (auto&& it1 : _Sources) 
    {  
        OrgasmChangeData& loc_ocd = it1.second;
        const float& loc_ar = loc_ocd.ArousalRate; 
        if (loc_ar == 0.0f) continue;

        float           loc_mult        = 0.0f;
        const uint32_t& loc_erozones    = loc_ocd.EroZones;

        for (auto && it2 : _PDATA.EroZones)
        {
            //use smalles value
            if ((it2.EroZoneSlot & loc_erozones) && (it2.Multiplier < loc_mult || loc_mult == 0.0f) && (it2.Multiplier > 0.0f)) 
            {
                loc_mult = it2.Multiplier;
            }
        }

        if ((loc_ocd.Mod & mArousingMovement) && (a_delta > 0.0f))
        {
            const auto loc_currentpos = _RDATA.Actor->GetPosition();
            const float loc_distance = loc_currentpos.GetSquaredDistance(_RDATA.lastpos);
            static const float loc_basedistance = loc_config->GetVariable<float>("Orgasm.fBaseDistance",2500.0f)*a_delta;

            if (loc_basedistance > 0.0f) loc_mult *= clamp(loc_distance/loc_basedistance,0.0f,2.0f);
        }

        //only if no edging is active
        if ((!(loc_ocd.Mod & (mEdgeOnly | mEdgeRandom)) || (GetOrgasmProgress(1) < loc_ocd.EdgeThreshold)) && loc_ocd.EdgeRemDuration <= 0.0f) 
        {
            loc_res += (loc_ar > 0.0f ? loc_ar*loc_mult : loc_ar);
        }
        else if (loc_ocd.EdgeRemDuration <= 0.0f) 
        {
            loc_ocd.EdgeRemDuration = loc_ocd.EdgeDuration; //set edge timeout
        }
    }
    return clamp(loc_res,-100.0f,100.0f);
}

inline float ORS::OrgasmActorData::CalculateArousalRateMult()
{
    float loc_res = 1.0f;
    for (auto&& it : _Sources) loc_res += it.second.ArousalRateMult;
    return clamp(loc_res,0.0f,100.0f);
}

inline float ORS::OrgasmActorData::CalculateHornyLevel(const float& a_delta)
{
    const float loc_orgprogp = _PDATA.OrgasmProgress/_RDATA.OrgasmCapacity;

    float loc_res = _PDATA.HornyLevel;

    if (_RDATA.OrgasmCount > 0)
    {
        if (loc_res > 85.0)
        {
            loc_res -= 3.0f*(100.0f/clamp(_RDATA.Arousal,50.0f,100.0f))*_RDATA.OrgasmCount*a_delta;
        }
        else
        {
            loc_res -= 1.0f*(clamp(_RDATA.Arousal,25.0f,100.0f))*_RDATA.OrgasmCount*a_delta;
        }
    }
    else
    {
        //base increase based on orgasm progress
        const float loc_rate = 4.0f*loc_orgprogp;
        if (loc_orgprogp > 0.05f) loc_res += loc_rate*a_delta;

        //aprox value to 100
        if (loc_orgprogp < 0.1f)
        {
            if (loc_res > 100.0f)
            {
                loc_res -= static_cast<float>(0.5f*(100.0f/clamp(_RDATA.Arousal,25.0f,100.0f))*a_delta);
                if (loc_res < 100.0f) loc_res = 100.0f;
            }
            else if (loc_res < 100.0f)
            {
                loc_res += static_cast<float>(0.5f*(_RDATA.Arousal/100.0f)*a_delta);
                if (loc_res > 100.0f) loc_res = 100.0f;
            }
        }
    }
    
    //validate horny level
    loc_res = clamp(loc_res,1.0,g_HornyLevels.back().Max);

    return loc_res;
}

void ORS::OrgasmActorData::ElapseChanges(const float& a_delta)
{
    std::vector<std::string> loc_toremove;

    for (auto&& [i_key,i_oc] : _Sources) 
    {   
        OrgasmChangeData& loc_oc = i_oc;

        //edge mod timeout
        if ((loc_oc.Mod & (mEdgeOnly | mEdgeRandom)) && (loc_oc.EdgeRemDuration > 0.0f))
        {
            
            loc_oc.EdgeRemDuration -= a_delta;

            if (loc_oc.EdgeRemDuration <= 0.0f)
            {
                loc_oc.EdgeRemDuration = 0.0f;
            }
        }

        if (loc_oc.Mod & mTimed)
        {
            loc_oc.ElapsedDuration += a_delta;
            if (loc_oc.ElapsedDuration >= loc_oc.Duration)
            {
                loc_toremove.push_back(i_key);
                continue;
            }
            else
            {
                if (loc_oc.OrgasmRate != 0.0f)
                {
                    if (loc_oc.Mod & mTimeMod_Lin)
                    {
                        loc_oc.OrgasmRate = std::lerp(loc_oc.OrgasmRateOriginal,0.0f,loc_oc.ElapsedDuration/loc_oc.Duration);
                    }
                    else if (loc_oc.Mod & mTimeMod_Exp)
                    {
                        loc_oc.OrgasmRate = std::lerp(loc_oc.OrgasmRateOriginal,0.0f,loc_oc.ElapsedDuration/loc_oc.Duration); //TODO
                    }
                    //if      (loc_oc.Mod & mTimeMod_Lin) loc_oc.OrgasmRate -= (loc_oc.OrgasmRateOriginal/loc_oc.Duration)*a_delta*(loc_oc.OrgasmRate > 0.0f ?  1.0f : -1.0f);
                    //else if (loc_oc.Mod & mTimeMod_Exp) loc_oc.OrgasmRate -= (loc_oc.OrgasmRateOriginal/loc_oc.Duration)*a_delta*(loc_oc.OrgasmRate > 0.0f ?  1.0f : -1.0f); //TODO - implement it properly. For now works as linear
                }

            }
        }
    }

    for (auto&& i_key : loc_toremove) _Sources.erase(i_key);
}

inline void ORS::OrgasmActorData::UpdateWidget()
{
    if (_RDATA.LinkedWidgetUsed)
    {
        static const float loc_widgetshow = Config::GetSingleton()->GetVariable<float>("Interface.fWidgetShowThreshold",0.025f);
        if (_RDATA.LinkedWidgetShown && (GetOrgasmProgress(1) < loc_widgetshow))
        {
            SendLinkedMeterEvent(wHide);
            _RDATA.LinkedWidgetShown = false;
        }
        else if (!_RDATA.LinkedWidgetShown && (GetOrgasmProgress(1) >= loc_widgetshow))
        {
            SendLinkedMeterEvent(wShow);
            _RDATA.LinkedWidgetShown = true;
        }
    }
}

inline void ORS::OrgasmActorData::UpdateExpression(const float& a_delta)
{
    static Config* loc_config = Config::GetSingleton();
    static const float loc_updtimePlayer = loc_config->GetVariable<float>("Interface.fExpressionUpdateTimePlayer",2.0f);
    static const float loc_updtimeNPC    = loc_config->GetVariable<float>("Interface.fExpressionUpdateTimeNPC",5.0f);

    if (_RDATA.Actor->Is3DLoaded())
    {
        _RDATA.ExpressionTimer += a_delta;

        const float loc_updtime = IsPlayer() ? loc_updtimePlayer : loc_updtimeNPC;

        if (_RDATA.OrgasmCount == 0)
        {
            const float loc_progress = GetOrgasmProgress(1);

            static const float loc_expupdmin   = loc_config->GetVariable<float>("Interface.fExpressionThresholdMin",0.05f);
            static const float loc_expupdmax   = loc_config->GetVariable<float>("Interface.fExpressionThresholdMax",0.1f);
            if ((!_RDATA.ExpressionSet || (_RDATA.ExpressionTimer >= loc_updtime)) && ((loc_progress > loc_expupdmax)))
            {
                SendOrgasmExpressionEvent(eNormalSet);
                _RDATA.ExpressionTimer = 0.0f;
                _RDATA.ExpressionSet = true;

            }
            else if (_RDATA.ExpressionSet && (loc_progress < loc_expupdmin)) 
            {
                SendOrgasmExpressionEvent(eNormalReset);
                _RDATA.ExpressionTimer = loc_updtime;
                _RDATA.ExpressionSet = false;
            }
        }
        else
        {
            if ((!_RDATA.ExpressionSet || (_RDATA.ExpressionTimer >= loc_updtime)))
            {
    
                SendOrgasmExpressionEvent(eOrgasmSet);
                _RDATA.ExpressionTimer = 0.0f;
                _RDATA.ExpressionSet = true;

            }
        }
    }
}

inline void ORS::OrgasmActorData::UpdatePosition()
{
    _RDATA.lastpos = _RDATA.Actor->GetPosition();
}

inline void ORS::OrgasmActorData::SendOrgasmEvent()
{
    if (_RDATA.Actor == nullptr) return;
    auto loc_handle = _RDATA.Actor->GetHandle();

    //copy, so correct unchanged data is send
    const RUNTIME_DATA loc_rdata = _RDATA;
    const PERSIST_DATA loc_pdata = _PDATA;

    SKSE::GetTaskInterface()->AddTask([loc_handle,loc_rdata,loc_pdata]
    {
        //LOG("Sending orgasm event for {}",loc_handle.get()->GetName())
        OrgasmEvents::GetSingleton()->OrgasmEvent.QueueEvent(loc_handle.get().get(),loc_rdata.OrgasmRate,loc_rdata.Arousal,loc_pdata.HornyLevel,loc_rdata.OrgasmCount);
    });
}

inline void ORS::OrgasmActorData::SendOrgasmExpressionEvent(ORS::ExpressionUpdateType a_type)
{
    if (_RDATA.Actor == nullptr) return;
    auto loc_handle = _RDATA.Actor->GetHandle();

    //copy, so correct unchanged data is send
    const RUNTIME_DATA loc_rdata = _RDATA;
    const PERSIST_DATA loc_pdata = _PDATA;

    SKSE::GetTaskInterface()->AddTask([loc_handle,loc_rdata,loc_pdata,a_type]
        {
            //LOG("Sending expression update for {}",loc_handle.get()->GetName())
            OrgasmEvents::GetSingleton()->ExpressionUpdateEvent.QueueEvent(loc_handle.get().get(),a_type,loc_rdata.OrgasmRate,loc_rdata.Arousal,loc_pdata.HornyLevel);
        }
    );
}

inline void ORS::OrgasmActorData::SendLinkedMeterEvent(LinkedWidgetUpdateType a_type)
{
    if (_RDATA.Actor == nullptr) return;
    auto loc_handle = _RDATA.Actor->GetHandle();
    SKSE::GetTaskInterface()->AddTask([loc_handle,a_type]
        {
            SKSE::ModCallbackEvent modEvent{
                "ORS_LinkedWidgetUpdate",
                "",
                (float)a_type,
                loc_handle.get().get()
            };

            if (loc_handle.get() == nullptr) 
            {
                return;
            }

            LOG("Sending linked widget update")

            auto modCallback = SKSE::GetModCallbackEventSource();
            modCallback->SendEvent(&modEvent);
        }
    );
}

inline bool ORS::OrgasmActorData::IsPlayer()
{
    static RE::Actor* loc_player = RE::PlayerCharacter::GetSingleton();
    return (_RDATA.Actor == loc_player);
}

inline void ORS::OrgasmActorData::CheckHornyLevel()
{
    static const bool loc_showmsg = Config::GetSingleton()->GetVariable<bool>("Interface.bHornyMessages",true);
    if (!loc_showmsg) return;

    const float loc_hornylevel = _PDATA.HornyLevel;
    for (auto&& it : g_HornyLevels)
    {
        const bool loc_inrange = (loc_hornylevel > it.Min && loc_hornylevel < it.Max);
        if (loc_inrange && !it.Printed)
        {
            it.Printed = true;
            auto loc_msg = it.Msg.c_str();
            SKSE::GetTaskInterface()->AddTask([loc_msg]
            {
                RE::DebugNotification(loc_msg);
            });
        }
        else if (!loc_inrange)
        {
            it.Printed = false;
        }
    }
}
