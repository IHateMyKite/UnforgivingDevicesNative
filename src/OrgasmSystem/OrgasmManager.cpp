#include <OrgasmSystem/OrgasmManager.h>

SINGLETONBODY(ORS::OrgasmManager)

namespace ORS
{
    inline const auto OrgasmSerData = _byteswap_ulong('ORSD');
}

void ORS::OrgasmManager::Setup()
{
    if (!_installed)
    {
        g_ArousalFaction = reinterpret_cast<RE::TESFaction*>(RE::TESDataHandler::GetSingleton()->LookupForm(0x1579C0,"UnforgivingDevices.esp"));
        _installed = true;
    }
    if(_installed) Update(0.0f);
}

void ORS::OrgasmManager::Update(float a_delta)
{
    std::unique_lock lock(_lock);
    if (a_delta <= 0.0f) return;
    auto loc_actors = UD::ActorSlotManager::GetSingleton()->GetRegisteredActors();

    //UDSKSELOG("OrgasmManager::Update({})",a_delta)

    for (auto&& it :_actors)
    {
        //UDSKSELOG("OrgasmManager::Update({}) - Updating actor {}",a_delta,it.first->GetName())

        OrgasmActorData* loc_actororgasm = &it.second;
        if (loc_actororgasm != nullptr && it.first != nullptr && (std::find(loc_actors.begin(),loc_actors.end(), it.first) != loc_actors.end()))
        {
            loc_actororgasm->SetActor(it.first);
            loc_actororgasm->Update(a_delta);
        }
        else
        {
            //error
        }
    }

    //UDSKSELOG("OrgasmManager::Update({}) - Done",a_delta)
}

bool ORS::OrgasmManager::AddOrgasmChange(RE::Actor* a_actor, std::string a_key, OrgasmMod a_mod, EroZone a_erozones, float a_orgasmRate, float a_orgasmRateMult, float a_orgasmForcing, float a_orgasmCapacity, float a_orgasmResisten, float a_orgasmResistenceMult)
{
    if (a_actor == nullptr) return false;
    std::unique_lock lock(_lock);
    return _actors[a_actor].AddOrgasmChange(a_key,a_mod,a_erozones,a_orgasmRate,a_orgasmRateMult,a_orgasmForcing,a_orgasmCapacity,a_orgasmResisten,a_orgasmResistenceMult);
}

bool ORS::OrgasmManager::AddOrgasmChange(RE::Actor* a_actor, std::string a_key, OrgasmMod a_mod, uint32_t a_erozones, float a_orgasmRate, float a_orgasmRateMult, float a_orgasmForcing, float a_orgasmCapacity, float a_orgasmResisten, float a_orgasmResistenceMult)
{
    if (a_actor == nullptr) return false;
    std::unique_lock lock(_lock);
    return _actors[a_actor].AddOrgasmChange(a_key,a_mod,(EroZone)a_erozones,a_orgasmRate,a_orgasmRateMult,a_orgasmForcing,a_orgasmCapacity,a_orgasmResisten,a_orgasmResistenceMult);
}

bool ORS::OrgasmManager::RemoveOrgasmChange(RE::Actor* a_actor, std::string a_key)
{
    if (a_actor == nullptr) return false;
    std::unique_lock lock(_lock);
    return _actors[a_actor].RemoveOrgasmChange(a_key);
}

bool ORS::OrgasmManager::UpdateOrgasmChangeVar(RE::Actor* a_actor, std::string a_key, OrgasmVariable a_variable, float a_value, OrgasmUpdateType a_mod)
{
    if (a_actor == nullptr) return false;
    std::unique_lock lock(_lock);
    return _actors[a_actor].UpdateOrgasmChangeVar(a_key,a_variable,a_value,a_mod);
}

float ORS::OrgasmManager::GetOrgasmChangeVar(RE::Actor* a_actor, std::string a_key, OrgasmVariable a_variable)
{
    if (a_actor == nullptr) return 0.0f;
    std::unique_lock lock(_lock);
    //UDSKSELOG("OrgasmManager::GetOrgasmChangeVar({},{},{})",a_actor->GetName(),a_key,a_variable)
    return _actors[a_actor].GetOrgasmChangeVar(a_key,a_variable);
}

bool ORS::OrgasmManager::HaveOrgasmChange(RE::Actor* a_actor, std::string a_key)
{
    if (a_actor == nullptr) return false;
    std::unique_lock lock(_lock);
    return _actors[a_actor].HaveOrgasmChange(a_key);
}

float ORS::OrgasmManager::GetOrgasmProgress(RE::Actor* a_actor, int a_mod)
{
    if (a_actor == nullptr) return 0.0f;
    std::unique_lock lock(_lock);
    return _actors[a_actor].GetOrgasmProgress(a_mod);
}

void ORS::OrgasmManager::ResetOrgasmProgress(RE::Actor* a_actor)
{
    if (a_actor == nullptr) return;
    std::unique_lock lock(_lock);
    _actors[a_actor].ResetOrgasmProgress();
}

float ORS::OrgasmManager::GetOrgasmVariable(RE::Actor* a_actor, OrgasmVariable a_variable)
{
    if (a_actor == nullptr) return 0.0f;
    std::unique_lock lock(_lock);
    //UDSKSELOG("OrgasmManager::GetOrgasmVariable({},{})",a_actor->GetName(),a_variable)
    return _actors[a_actor].GetOrgasmVariable(a_variable);
}

float ORS::OrgasmManager::GetAntiOrgasmRate(RE::Actor* a_actor)
{
    if (a_actor == nullptr) return 0.0f;
    std::unique_lock lock(_lock);
    return _actors[a_actor].GetAntiOrgasmRate();
}

void ORS::OrgasmManager::LinkActorToMeter(RE::Actor* a_actor, std::string a_path, MeterWidgetType a_type, int a_id)
{
    if (a_actor == nullptr) return;
    std::unique_lock lock(_lock);
    return _actors[a_actor].LinkActorToMeter(a_path,a_type,a_id);
}

void ORS::OrgasmManager::UnlinkActorFromMeter(RE::Actor* a_actor)
{
    if (a_actor == nullptr) return;
    std::unique_lock lock(_lock);
    return _actors[a_actor].UnlinkActorFromMeter();
}

void ORS::OrgasmManager::RegisterPapyrusFunctions(RE::BSScript::IVirtualMachine *vm)
{
    #define REGISTERPAPYRUSFUNC(name,unhook) vm->RegisterFunction(#name, "OrgasmSystem", ORS::name,unhook);
    // ----
    REGISTERPAPYRUSFUNC(AddOrgasmChange,true)
    REGISTERPAPYRUSFUNC(RemoveOrgasmChange,true)
    REGISTERPAPYRUSFUNC(GetOrgasmChangeVar,true)
    REGISTERPAPYRUSFUNC(HaveOrgasmChange,true)
    REGISTERPAPYRUSFUNC(GetOrgasmProgress,true)
    REGISTERPAPYRUSFUNC(GetOrgasmVariable,true)  
    REGISTERPAPYRUSFUNC(GetAntiOrgasmRate,true)
    REGISTERPAPYRUSFUNC(ResetOrgasmProgress,true)
    REGISTERPAPYRUSFUNC(LinkActorToMeter,true)
    REGISTERPAPYRUSFUNC(UnlinkActorFromMeter,true)
    // ----
    #undef REGISTERPAPYRUSFUNC
}

void ORS::OrgasmManager::OnGameLoaded(SKSE::SerializationInterface* serde)
{
    uint32_t loc_type;
    uint32_t loc_size;
    uint32_t loc_version;

    while (serde->GetNextRecordInfo(loc_type, loc_version, loc_size)) 
    {
        if (loc_type == OrgasmSerData) 
        {
            size_t loc_actornum = 0;
            serde->ReadRecordData(&loc_actornum, sizeof(size_t));

            for (size_t i = 0; i < loc_actornum; i++)
            {
                RE::FormID loc_actorFormID;
                serde->ReadRecordData(&loc_actorFormID, sizeof(RE::FormID));

                RE::FormID loc_newActorFormID;
                if (!serde->ResolveFormID(loc_actorFormID, loc_newActorFormID)) {
                    UDSKSELOG("Actor ID {:X} could not be found after loading the save.", loc_actorFormID)
                    continue;
                }

                RE::Actor* loc_actor = RE::TESForm::LookupByID<RE::Actor>(loc_newActorFormID);

                UDSKSELOG("Loaded actor {} from save",loc_actor->GetName())
                _actors[loc_actor] = OrgasmActorData();
                _actors[loc_actor].SetActor(loc_actor);
                _actors[loc_actor].OnGameLoaded(serde);
            }
        }
    }

}

void ORS::OrgasmManager::OnGameSaved(SKSE::SerializationInterface* serde)
{
    std::unique_lock lock(_lock);

    if (!serde->OpenRecord(OrgasmSerData, 0)) {
        return;
    }

    const size_t loc_actornum = _actors.size();
    serde->WriteRecordData(&loc_actornum,sizeof(size_t)); //first number of actors

    //now iterate thru all actors
    for (auto&& it : _actors)
    {
        OrgasmActorData loc_od = it.second;
        RE::FormID loc_formid = it.first->GetFormID();
        serde->WriteRecordData(&loc_formid,sizeof(RE::FormID));
        UDSKSELOG("Saving actor {}",it.first->GetName())
        loc_od.OnGameSaved(serde);
    }

}

void ORS::OrgasmManager::OnRevert(SKSE::SerializationInterface* serde)
{
    std::unique_lock lock(_lock);

    for (auto&& it : _actors)
    {
        it.second.OnRevert(serde);
    }

    _actors.clear();
}