#include <OrgasmSystem/OrgasmData.h>
#include <boost/algorithm/clamp.hpp>

RE::TESFaction* ORS::g_ArousalFaction = nullptr;

using boost::algorithm::clamp;

void ORS::OrgasmActorData::Update(float a_delta)
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
    _AntiOrgasmRate         = static_cast<float>(_ArousalMult*_OrgasmResistenceMult*(_OrgasmProgress*(_OrgasmResistence/100.0))*a_delta);

    if (_OrgasmRate*_OrgasmRateMult > 0.0f)
    {
        _OrgasmProgress -= _AntiOrgasmRate;
    }
    else
    {
        _OrgasmProgress -= 3*_AntiOrgasmRate;
    }

    _OrgasmProgress = clamp(_OrgasmProgress,0.0,_OrgasmCapacity);
    if (_OrgasmProgress < 0.01f) _OrgasmProgress = 0.0f;

    UDSKSELOG("ORS::OrgasmActorData::Update({}) - Orgasm progress = {}",_actor->GetName(),_OrgasmProgress)
}

float ORS::OrgasmActorData::GetOrgasmProgress() const
{
    return _OrgasmProgress;
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
    if (OrgasmChangeExist(a_key)) return false;
    a_key.copy(_Sources[a_key].Key,a_key.size() + 1); //+ 1 so it also copy \0 character
    _Sources[a_key].OrgasmRate              = a_orgasmRate;
    _Sources[a_key].OrgasmRateMult          = a_orgasmRateMul;
    _Sources[a_key].OrgasmForcing           = a_orgasmForcing;
    _Sources[a_key].OrgasmCapacity          = a_orgasmCapacity;
    _Sources[a_key].OrgasmResistence        = a_orgasmResistence;
    _Sources[a_key].OrgasmResistenceMult    = a_orgasmResistenceMult;
    _Sources[a_key].Mod                     = a_mod;
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
        default:
            return 0.0f;
    };
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

        for (auto && it2 : _EroZones)
        {
            if (it2.EroZoneSlot & loc_erozones) loc_mult += it2.Multiplier;
        }

        //only if no edging is active
        if (!(it1.second.Mod & 0x01)) loc_res += (it1.second.OrgasmRate*loc_mult);
    }
    return clamp(loc_res,-5000.0f,5000.0f);
}

float ORS::OrgasmActorData::CalculateOrgasmRateMult()
{
    float loc_res = 1.0f;
    for (auto&& it1 : _Sources) 
    {   
        float       loc_mult        = 0.0f;
        uint32_t    loc_erozones    = it1.second.EroZones;

        for (auto && it2 : _EroZones)
        {
            if (it2.EroZoneSlot & loc_erozones) loc_mult += it2.Multiplier;
        }

        //only if no edging is active
        if (!(it1.second.Mod & 0x01)) loc_res += (it1.second.OrgasmRateMult*loc_mult);
    }
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
    float loc_res = _OrgasmCapacityDef;
    for (auto&& it : _Sources) loc_res += it.second.OrgasmCapacity;
    return clamp(loc_res,1.0f,10000.0f);
}

float ORS::OrgasmActorData::CalculateOrgasmResistence()
{
    float loc_res = _OrgasmResistenceDef;
    for (auto&& it1 : _Sources) 
    {   
        float       loc_mult        = 0.0f;
        uint32_t    loc_erozones    = it1.second.EroZones;

        for (auto && it2 : _EroZones)
        {
            if (it2.EroZoneSlot & loc_erozones) loc_mult += it2.Multiplier;
        }

        //only if no edging is active
        if (!(it1.second.Mod & 0x01)) loc_res += (it1.second.OrgasmResistence*loc_mult);
    }
    return clamp(loc_res,0.0f,100.0f);
}

float ORS::OrgasmActorData::CalculateOrgasmResistenceMult()
{
    float loc_res = 1.0f;
    for (auto&& it1 : _Sources) 
    {   
        float       loc_mult        = 0.0f;
        uint32_t    loc_erozones    = it1.second.EroZones;

        for (auto && it2 : _EroZones)
        {
            if (it2.EroZoneSlot & loc_erozones) loc_mult += it2.Multiplier;
        }

        //only if no edging is active
        if (!(it1.second.Mod & 0x01)) loc_res += (it1.second.OrgasmResistenceMult*loc_mult);
    }
    return clamp(loc_res,0.0f,100.0f);
}
