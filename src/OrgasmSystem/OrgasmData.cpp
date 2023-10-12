#include <OrgasmSystem/OrgasmData.h>
#include <UD_UI.h>

RE::TESFaction* ORS::g_ArousalFaction = nullptr;

using boost::algorithm::clamp;

void ORS::OrgasmActorData::Update(const float& a_delta)
{
    if (_actor == nullptr) return;

    _Arousal                = static_cast<int8_t>(_actor->GetFactionRank(g_ArousalFaction,_actor->IsPlayerRef()));
    _OrgasmRate             = CalculateOrgasmRate();
    _OrgasmRateMult         = CalculateOrgasmRateMult();
    _OrgasmForcing          = CalculateOrgasmForcing();
    _OrgasmCapacity         = CalculateOrgasmCapacity();
    _OrgasmResistence       = CalculateOrgasmResistence();
    _OrgasmResistenceMult   = CalculateOrgasmResistenceMult();

    _OrgasmProgress        += CalculateOrgasmProgress()*a_delta;

    
    float _ArousalMult      = clamp(std::pow(10.0f,clamp(100.0f/clamp(_Arousal,1,100),1.0f,2.0f) - 1.0f),1.0,100.0);;
    _AntiOrgasmRate         = static_cast<float>(_ArousalMult*_OrgasmResistenceMult*_OrgasmResistence);

    if (_OrgasmRate*_OrgasmRateMult > 0.0f)
    {
        _OrgasmProgress -= _AntiOrgasmRate*_OrgasmProgress*a_delta/100.0;
        _OrgasmRateTotal = _OrgasmRate*_OrgasmRateMult - _AntiOrgasmRate;
    }
    else
    {
        _OrgasmProgress -= 3*_AntiOrgasmRate*_OrgasmProgress*a_delta/100.0;
        _OrgasmRateTotal = _OrgasmRate*_OrgasmRateMult - 3*_AntiOrgasmRate;
    }

    _OrgasmProgress = clamp(_OrgasmProgress,0.0,_OrgasmCapacity);
    if (_OrgasmProgress < 0.01f) _OrgasmProgress = 0.0f;


    ElapseChanges(a_delta);

    UpdateWidget();

    UDSKSELOG("ORS::OrgasmActorData::Update({}) - {} = {} - {} --- T= {}",_actor->GetName(),_OrgasmProgress,_OrgasmRate,_AntiOrgasmRate,_OrgasmRateTotal)
}

float ORS::OrgasmActorData::GetOrgasmProgress(int a_mod) const
{
    if (a_mod == 0) return _OrgasmProgress;
    else return _OrgasmProgress/_OrgasmCapacity;
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
            uint16_t loc_suf = 0;
            //find unused key
            while (_Sources.find(a_key + std::to_string(loc_suf)) != _Sources.end())
            {
                loc_suf += 1;
            }
            //add sufix to key and use it instead
            a_key += std::to_string(loc_suf);
        }
        else
        {
            return false;
        }
        
    }
    UDSKSELOG("OrgasmActorData::AddOrgasmChange({},{},{},{},{})",_actor->GetName(),a_key,a_mod,a_erozones,a_orgasmRate)
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
    return _Sources.erase(a_key) > 0;
}

bool ORS::OrgasmActorData::UpdateOrgasmChangeVar(std::string a_key, OrgasmVariable a_variable, float a_value, OrgasmUpdateType a_mod)
{
    if (!OrgasmChangeExist(a_key)) return false;
    
    float* loc_var = nullptr;

    switch (a_variable)
    {
        case vOrgasmRate:
            loc_var = &_Sources[a_key].OrgasmRate;
            break;
        case vOrgasmRateMult:
            loc_var = &_Sources[a_key].OrgasmRateMult;
            break;
        case vOrgasmResistence:
            loc_var = &_Sources[a_key].OrgasmResistence;
            break;
        case vOrgasmResistenceMult:
            loc_var = &_Sources[a_key].OrgasmResistenceMult;
            break;
        case vOrgasmCapacity:
            loc_var = &_Sources[a_key].OrgasmCapacity;
            break;
        case vOrgasmForcing:
            loc_var = &_Sources[a_key].OrgasmForcing;
            break;
        case vElapsedTime:
            loc_var = &_Sources[a_key].ElapsedDuration;
            break;
        default:
            return false;
    };

    switch (a_mod)
    {
        case mSet:
            *loc_var = a_value;
            return true;
        case mAdd:
            *loc_var += a_value;
            return true;
        case mMultiply: 
            *loc_var *= a_value;
            return true;
        default:
            return false;
    }

    return false;
}

float ORS::OrgasmActorData::GetOrgasmChangeVar(std::string a_key, OrgasmVariable a_variable)
{
    if (!OrgasmChangeExist(a_key)) return 0.0f;
    
    switch (a_variable)
    {
        case vOrgasmRate:
            return _Sources[a_key].OrgasmRate;
        case vOrgasmRateMult:
            return _Sources[a_key].OrgasmRateMult;
        case vOrgasmResistence:
            return _Sources[a_key].OrgasmResistence;
        case vOrgasmResistenceMult:
            return _Sources[a_key].OrgasmResistenceMult;
        case vOrgasmCapacity:
            return _Sources[a_key].OrgasmCapacity;
        case vOrgasmForcing:
            return _Sources[a_key].OrgasmForcing;
        case vElapsedTime:
            return _Sources[a_key].ElapsedDuration;
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
        case vOrgasmRate:
            return CalculateOrgasmRate();
        case vOrgasmRateMult:
            return CalculateOrgasmRateMult();
        case vOrgasmResistence:
            return CalculateOrgasmResistence();
        case vOrgasmResistenceMult:
            return CalculateOrgasmResistenceMult();
        case vOrgasmCapacity:
            return CalculateOrgasmCapacity();
        case vOrgasmForcing:
            return CalculateOrgasmForcing();
        default:
            return 0.0f;
    };
}

void ORS::OrgasmActorData::LinkActorToMeter(std::string a_path, MeterWidgetType a_type, int a_id)
{
    _LinkedWidgetPath   = a_path;
    _LinkedWidgetType   = a_type;
    _LinkedWidgetId     = a_id; 

    switch (a_type)
    {
        case tSkyUi : UD::MeterManager::SetExtCalcSkyUi(_LinkedWidgetPath,this,&OrgasmActorData::GetOrgasmProgressLink); break;
        case tIWW   : UD::MeterManager::SetExtCalcIWW(a_id,this,&OrgasmActorData::GetOrgasmProgressLink); break;
    }
    
}

void ORS::OrgasmActorData::UnlinkActorFromMeter()
{
    switch (_LinkedWidgetType)
    {
        case tSkyUi : UD::MeterManager::UnsetExtCalcSkyUi(_LinkedWidgetPath); break;
        case tIWW   : UD::MeterManager::UnsetExtCalcIWW(_LinkedWidgetId); break;
    }

    _LinkedWidgetPath   = "";
    _LinkedWidgetType   = tSkyUi;
    _LinkedWidgetId     = 0; 
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
    serde->ReadRecordData(&_OrgasmProgress,sizeof(float));

    serde->ReadRecordData(_EroZones,32*sizeof(OrgasmEroZone));

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

void ORS::OrgasmActorData::OnGameSaved(SKSE::SerializationInterface* serde)
{
    serde->WriteRecordData(&_OrgasmProgress,sizeof(float));

    serde->WriteRecordData(_EroZones,32*sizeof(OrgasmEroZone));

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

float ORS::OrgasmActorData::CalculateOrgasmProgress()
{
    return _OrgasmRate*_OrgasmRateMult;
}

float ORS::OrgasmActorData::CalculateOrgasmRate()
{
    float loc_res = 0.0f;
    for (auto&& it1 : _Sources) 
    {   
        float       loc_mult        = 0.0f;
        uint32_t    loc_erozones    = it1.second.EroZones;

        if (it1.second.OrgasmRate > 0.0f)
        {
            for (auto && it2 : _EroZones)
            {
                if (it2.EroZoneSlot & loc_erozones) loc_mult += it2.Multiplier;
            }
        }
        //only if no edging is active
        if (!(it1.second.Mod & 0x01) || (GetOrgasmProgress(1) < 0.9f)) loc_res += (it1.second.OrgasmRate*loc_mult);
    }
    return clamp(loc_res,-5000.0f,5000.0f);
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

void ORS::OrgasmActorData::ElapseChanges(const float& a_delta)
{
    for (auto&& it : _Sources) 
    {   
        if (it.second.Mod & mTimed)
        {
            it.second.ElapsedDuration += a_delta;
            if (it.second.ElapsedDuration > it.second.Duration)
            {
                UDSKSELOG("OrgasmActorData::ElapseChanges({}) - Change {} removed because time elapsed",a_delta,it.first)
                _Sources.erase(it.first);
            }
            else
            {
                if (it.second.Mod & mTimeMod_Lin) it.second.OrgasmRate -= (it.second.OrgasmRateOriginal/it.second.Duration)*a_delta;
                else if (it.second.Mod & mTimeMod_Exp) it.second.OrgasmRate -= (it.second.OrgasmRateOriginal/it.second.Duration)*a_delta; //TODO - implement it properly

                //if (it.second.OrgasmRate < 0.0f) it.second.OrgasmRate = 0.0f;
            }
        }
    }
}

inline void ORS::OrgasmActorData::UpdateWidget()
{
    //if (_LinkedWidgetPath == "") return;
    //
    //const float loc_rate = _OrgasmRateTotal*100.0/_OrgasmCapacity;
    //
    //if (_LinkedWidgetType == tSkyUi)
    //{
    //    UD::MeterManager::SetMeterRateSkyUi(_LinkedWidgetPath,loc_rate);
    //}
    //else if (_LinkedWidgetType == tIWW)
    //{
    //    UD::MeterManager::SetMeterRateIWW(_LinkedWidgetId,loc_rate);
    //}
}
