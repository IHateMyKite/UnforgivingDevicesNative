#include <OrgasmSystem/OrgasmData.h>
#include <UD_UI.h>
#include <UD_ModEvents.h>

using boost::algorithm::clamp;

void ORS::OrgasmActorData::Update(const float& a_delta)
{
    if (_actor == nullptr) return;

    if (_RDATA.OrgasmTimeout > 0.0f)
    {
        _RDATA.OrgasmTimeout -= a_delta;
        return;
    }

    _RDATA.ArousalRate            = CalculateArousalRate(a_delta);
    _RDATA.ArousalRateMult        = CalculateArousalRateMult();

    const float loc_da            = _RDATA.ArousalRate*_RDATA.ArousalRateMult*a_delta;

    //UDSKSELOG("OrgasmActorData::Update({},{}) _Arousal before = {}, rate = {}",_actor->GetName(),a_delta,_Arousal,loc_da)
    if (OSLAModifyArousal != nullptr) _RDATA.Arousal = OSLAModifyArousal(_actor,loc_da);
    //UDSKSELOG("OrgasmActorData::Update({},{}) _Arousal after = {}, rate = {}",_actor->GetName(),a_delta,_Arousal,loc_da)

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

    //validate horny level
    _PDATA.HornyLevel = clamp(_PDATA.HornyLevel,1.0,400.0);

    //UDSKSELOG("ORS::OrgasmActorData::Update({}) - {} = {} - {} --- T= {}",_actor->GetName(),_OrgasmProgress,_OrgasmRate,_AntiOrgasmRate,_OrgasmRateTotal)
}

float ORS::OrgasmActorData::GetOrgasmProgress(int a_mod) const
{
    if (a_mod == 0) return _PDATA.OrgasmProgress;
    else return _RDATA.OrgasmCapacity > 0.0f ? _PDATA.OrgasmProgress/_RDATA.OrgasmCapacity : 0.0f;
}

void ORS::OrgasmActorData::ResetOrgasmProgress()
{
    UDSKSELOG("OrgasmManager::ResetOrgasmProgress({})",_actor->GetName())
    //std::unique_lock lock(_lock);
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
    //std::unique_lock lock(_lock);
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

    UDSKSELOG("OrgasmActorData::AddOrgasmChange({},{},{})",_actor->GetName(),a_key,a_orgasmRate)

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
    //std::unique_lock lock(_lock);
    UDSKSELOG("OrgasmActorData::RemoveOrgasmChange({},{})",_actor->GetName(),a_key)
    return _Sources.erase(a_key) > 0;
}

bool ORS::OrgasmActorData::UpdateOrgasmChangeVar(std::string a_key, OrgasmVariable a_variable, float a_value, OrgasmUpdateType a_mod)
{
    //std::unique_lock lock(_lock);
    if (!OrgasmChangeExist(a_key))
    {
        UDSKSELOG("OrgasmActorData::UpdateOrgasmChangeVar({},{},{},{}) cant find key!",a_key,a_variable,a_value,a_mod)
        return false;
    }

    float* loc_var = nullptr;

    //UDSKSELOG("OrgasmActorData::UpdateOrgasmChangeVar({},{},{},{})",a_key,a_variable,a_value,a_mod)

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
            UDSKSELOG("OrgasmActorData::UpdateOrgasmChangeVar({},{},{},{}) - Set to {}",a_key,a_variable,a_value,a_mod,*loc_var)
            return true;
        case mAdd:
            *loc_var += a_value;
            if (a_variable == vOrgasmRate) _Sources[a_key].OrgasmRateOriginal += a_value;
            UDSKSELOG("OrgasmActorData::UpdateOrgasmChangeVar({},{},{},{}) - Increased to {}",a_key,a_variable,a_value,a_mod,*loc_var)
            return true;
        case mMultiply: 
            *loc_var *= a_value;
            if (a_variable == vOrgasmRate) _Sources[a_key].OrgasmRateOriginal *= a_value;
            UDSKSELOG("OrgasmActorData::UpdateOrgasmChangeVar({},{},{},{}) - Multiplied to {}",a_key,a_variable,a_value,a_mod,*loc_var)
            return true;
        default:
            return false;
    }

    return false;
}

float ORS::OrgasmActorData::GetOrgasmChangeVar(std::string a_key, OrgasmVariable a_variable)
{
    //std::unique_lock lock(_lock);
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
    //std::unique_lock lock(_lock);
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
        case tSkyUi : UD::MeterManager::SetExtCalcSkyUi(_RDATA.LinkedWidgetPath,this,&OrgasmActorData::GetOrgasmProgressLink); break;
        case tIWW   : UD::MeterManager::SetExtCalcIWW(a_id,this,&OrgasmActorData::GetOrgasmProgressLink); break;
    }
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
    _RDATA.OrgasmTimeout += 5.0f;

    ResetOrgasmProgress();

    std::string loc_key = MakeUniqueKey("PostOrgasm");

    AddOrgasmChange(loc_key,(OrgasmMod)0xA000C,eDefault,-5.0f,-0.25f,0.0f,0.0f,0.25f,0.25f);
    UpdateOrgasmChangeVar(loc_key, vArousalRate, -15.0f, mSet);

    SendOrgasmEvent();

    _PDATA.HornyLevel -= (_PDATA.HornyLevel > 100.0 ? (_PDATA.HornyLevel*0.25f + 25.0f) : 40.0f);

    if (_RDATA.LinkedWidgetUsed) SendLinkedMeterEvent(wHide);
}

RE::Actor* ORS::OrgasmActorData::GetActor()
{
    return _actor;
}

void ORS::OrgasmActorData::SetActor(RE::Actor* a_actor)
{
    _actor = a_actor;
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
        UDSKSELOG("Not loading cosave data because data version is old!")
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

    loc_res += (_RDATA.OrgasmRate*_RDATA.OrgasmRateMult*a_delta*(clamp(_PDATA.HornyLevel/100.0f,0.5f,4.0f)));

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
        loc_res -= static_cast<float>(3*_RDATA.AntiOrgasmRate*loc_res*a_delta/100.0f);
        _RDATA.OrgasmRateTotal = loc_OrgasmRateAfterMult - 3*_RDATA.AntiOrgasmRate;
    }

    loc_res = clamp(loc_res,0.0f,_RDATA.OrgasmCapacity);
    if (loc_res < 0.01f) loc_res = 0.0f;

    return loc_res;
}

float ORS::OrgasmActorData::CalculateOrgasmRate(const float& a_delta)
{
    static OrgasmConfig* loc_config = OrgasmConfig::GetSingleton();
    float loc_res = 0.0f;
    for (auto&& it1 : _Sources) 
    {   
        OrgasmChangeData& loc_ocd = it1.second;
        const float& loc_or = loc_ocd.OrgasmRate;
        if (loc_or == 0.0f) continue;

        float           loc_mult        = 0.0f;
        const uint32_t& loc_erozones    = loc_ocd.EroZones;

        for (auto && it2 : _PDATA.EroZones)
        {
            //use biggest value
            if ((it2.EroZoneSlot & loc_erozones) && (it2.Multiplier > loc_mult)) 
            {
                loc_mult = it2.Multiplier;
            }
        }

        if ((loc_ocd.Mod & mArousingMovement) && (a_delta > 0.0f))
        {
            {
                const auto loc_currentpos = _actor->GetPosition();
                const float loc_distance = loc_currentpos.GetSquaredDistance(_RDATA.lastpos);
                const float loc_basedistance = loc_config->BASEDISTANCE*a_delta;

                if (loc_basedistance > 0.0f) loc_mult *= clamp(loc_distance/loc_basedistance,0.0f,3.0f);

                //UDSKSELOG("OrgasmActorData::CalculateOrgasmRate() - Distance= {}, Base distance = {}, Distance multiplier = {}",loc_distance,loc_basedistance,loc_distance/loc_basedistance)
            }
        }

        //only if no edging is active
        if ((!(loc_ocd.Mod & (mEdgeOnly | mEdgeRandom)) || (GetOrgasmProgress(1) < loc_ocd.EdgeThreshold)) && loc_ocd.EdgeRemDuration <= 0.0f) 
        {
            //UDSKSELOG("OrgasmActorData::CalculateOrgasmRate({},{}) - Changing orgasm rate by {} - Mult = {}",_actor->GetName(),it1.first,it1.second.OrgasmRate*loc_mult,loc_mult)
            loc_res += (loc_or > 0.0f ? loc_or*loc_mult : loc_or);
        }
        else if (loc_ocd.EdgeRemDuration <= 0.0f) 
        {
            loc_ocd.EdgeRemDuration = loc_ocd.EdgeDuration; //set edge timeout
        }
    }

    loc_res = clamp(loc_res,-5000.0f,5000.0f);
    //UDSKSELOG("OrgasmActorData::CalculateOrgasmRate({}) - Final orgasm rate = {} ",_actor->GetName(),loc_res)

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
    static OrgasmConfig* loc_config = OrgasmConfig::GetSingleton();
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
            //use biggest value
            if ((it2.EroZoneSlot & loc_erozones) && (it2.Multiplier > loc_mult)) 
            {
                loc_mult = it2.Multiplier;
            }
        }

        if ((loc_ocd.Mod & mArousingMovement) && (a_delta > 0.0f))
        {
            const auto loc_currentpos = _actor->GetPosition();
            const float loc_distance = loc_currentpos.GetSquaredDistance(_RDATA.lastpos);
            const float loc_basedistance = loc_config->BASEDISTANCE*a_delta;

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

    //base increase based on orgasm progress
    if (loc_orgprogp > 0.05f) loc_res += clamp(5*loc_orgprogp,1.0f,5.0f)*a_delta;

    //aprox value to 100
    if (loc_res > 100.0f)
    {
        loc_res -= static_cast<float>(1.5*(100.0/clamp(_RDATA.Arousal,25.0,100.0))*a_delta);
        if (loc_res < 100.0f) loc_res = 100.0f;
    }
    else if (loc_res < 100.0f)
    {
        loc_res += static_cast<float>(1.0f*(_RDATA.Arousal/100.0f)*a_delta);
        if (loc_res > 100.0f) loc_res = 100.0f;
    }
    return loc_res;
}

void ORS::OrgasmActorData::ElapseChanges(const float& a_delta)
{
    for (auto&& it : _Sources) 
    {   
        OrgasmChangeData& loc_oc = it.second;

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
                _Sources.erase(it.first);
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
                        if (loc_oc.OrgasmRate > 0.0f) loc_oc.OrgasmRate = std::lerp(0.0f,loc_oc.OrgasmRateOriginal,loc_oc.ElapsedDuration/loc_oc.Duration);
                        else loc_oc.OrgasmRate = std::lerp(loc_oc.OrgasmRateOriginal,0.0f,loc_oc.ElapsedDuration/loc_oc.Duration);
                    }
                    //if      (loc_oc.Mod & mTimeMod_Lin) loc_oc.OrgasmRate -= (loc_oc.OrgasmRateOriginal/loc_oc.Duration)*a_delta*(loc_oc.OrgasmRate > 0.0f ?  1.0f : -1.0f);
                    //else if (loc_oc.Mod & mTimeMod_Exp) loc_oc.OrgasmRate -= (loc_oc.OrgasmRateOriginal/loc_oc.Duration)*a_delta*(loc_oc.OrgasmRate > 0.0f ?  1.0f : -1.0f); //TODO - implement it properly. For now works as linear
                }

            }
        }
    }
}

inline void ORS::OrgasmActorData::UpdateWidget()
{
    static OrgasmConfig* loc_config = OrgasmConfig::GetSingleton();
    if (_RDATA.LinkedWidgetUsed)
    {
        if (_RDATA.LinkedWidgetShown && (GetOrgasmProgress(1) < loc_config->WIDGETSHOWTH))
        {
            SendLinkedMeterEvent(wHide);
            _RDATA.LinkedWidgetShown = false;
        }
        else if (!_RDATA.LinkedWidgetShown && (GetOrgasmProgress(1) >= loc_config->WIDGETSHOWTH))
        {
            SendLinkedMeterEvent(wShow);
            _RDATA.LinkedWidgetShown = true;
        }
    }
}

inline void ORS::OrgasmActorData::UpdateExpression(const float& a_delta)
{
    static OrgasmConfig* loc_config = OrgasmConfig::GetSingleton();
    if (_actor->Is3DLoaded())
    {
        _RDATA.ExpressionTimer += a_delta;

        const float loc_progress = GetOrgasmProgress(1);

        if ((!_RDATA.ExpressionSet || (_RDATA.ExpressionTimer >= loc_config->EXPRUPDATETIME)) && (loc_progress > loc_config->EXPUPDATEMAXTH))
        {
    
            SendOrgasmExpressionEvent(eSet);
            _RDATA.ExpressionTimer = 0.0f;
            _RDATA.ExpressionSet = true;

        }
        else if (_RDATA.ExpressionSet && (loc_progress < loc_config->EXPUPDATEMINTH)) 
        {
            SendOrgasmExpressionEvent(eReset);
            _RDATA.ExpressionTimer = loc_config->EXPRUPDATETIME;
            _RDATA.ExpressionSet = false;
        }
    }
}

inline void ORS::OrgasmActorData::UpdatePosition()
{
    _RDATA.lastpos = _actor->GetPosition();
}

inline void ORS::OrgasmActorData::SendOrgasmEvent()
{
    if (_actor == nullptr) return;
    auto loc_handle = _actor->GetHandle();
    SKSE::GetTaskInterface()->AddTask([loc_handle,this]
        {
            UDSKSELOG("Sending orgasm event for {}",loc_handle.get()->GetName())
            UD::ModEvents::GetSingleton()->OrgasmEvent.QueueEvent(loc_handle.get().get(),_RDATA.OrgasmRate,_RDATA.Arousal);
        }
    );
}

inline void ORS::OrgasmActorData::SendOrgasmExpressionEvent(ORS::ExpressionUpdateType a_type)
{
    if (_actor == nullptr) return;

    UDSKSELOG("OrgasmActorData::SendOrgasmExpressionEvent start")
    //UDSKSELOG("OrgasmActorData::SendOrgasmExpressionEvent({},{})",_actor->GetName(),a_type)

    auto loc_handle = _actor->GetHandle();
    SKSE::GetTaskInterface()->AddTask([loc_handle,a_type]
        {
            SKSE::ModCallbackEvent modEvent{
                "ORS_ExpressionUpdate",
                "",
                (float)a_type,
                loc_handle.get().get()
            };

            UDSKSELOG("Sending orgasm exp update")

            if (loc_handle.get() == nullptr) 
            {
                return;
            }
            auto modCallback = SKSE::GetModCallbackEventSource();
            modCallback->SendEvent(&modEvent);
        }
    );
    UDSKSELOG("OrgasmActorData::SendOrgasmExpressionEvent end")
}

inline void ORS::OrgasmActorData::SendLinkedMeterEvent(LinkedWidgetUpdateType a_type)
{
    UDSKSELOG("OrgasmActorData::SendLinkedMeterEvent start")
    if (_actor == nullptr) return;
    auto loc_handle = _actor->GetHandle();
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

            UDSKSELOG("Sending linked widget update")

            auto modCallback = SKSE::GetModCallbackEventSource();
            modCallback->SendEvent(&modEvent);
        }
    );
    UDSKSELOG("OrgasmActorData::SendLinkedMeterEvent end")
}
