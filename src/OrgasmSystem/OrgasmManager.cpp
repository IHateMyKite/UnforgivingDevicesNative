#include <OrgasmSystem/OrgasmManager.h>

SINGLETONBODY(ORS::OrgasmManager)

namespace ORS
{
    inline const auto OrgasmSerData = _byteswap_ulong('ORSD');
    ModifyArousal OSLAModifyArousal;
}

void ORS::OrgasmManager::Setup()
{
    ORS::Config::GetSingleton()->Setup();
    if (!_installed)
    {
        //g_ArousalFaction = reinterpret_cast<RE::TESFaction*>(RE::TESDataHandler::GetSingleton()->LookupForm(0x1579C0,"UnforgivingDevices.esp"));
        HINSTANCE dllHandle = LoadLibrary(TEXT("OSLAroused.dll"));
        if (dllHandle != NULL)
        {
            FARPROC pModifyArousal = GetProcAddress(HMODULE (dllHandle),"ModifyArousalExt");
            OSLAModifyArousal = ModifyArousal(pModifyArousal);
            LOG("OrgasmManager::Setup() - Modify arousal imported")
            //FreeLibrary(dllHandle);
        }
        else
        {
            ERROR("OrgasmManager::Setup() - ERROR: Cant find OSLAroused.dll!!")
        }
        _installed = true;
        LOG("OrgasmConfig::Setup - installed")
    }
    if(_installed) Update(0.0f);
}

void ORS::OrgasmManager::Update(float a_delta)
{
    UniqueLock lock(_lock);
    if (a_delta <= 0.0f) return;

    //LOG("OrgasmManager::Update({})",a_delta)

    std::vector<uint32_t> loc_toremove;
    for (auto&& it :_actors)
    {
        RE::Actor* loc_actor = RE::Actor::LookupByHandle(it.first).get();
        //check if actor is dead. If yes, unregister it
        if (loc_actor && loc_actor->IsDead())
        {
            loc_toremove.push_back(it.first);
        }
    }
    for (auto&& it : loc_toremove) _actors.erase(it); //remove invalid actors

    
    std::vector<std::thread> loc_threads;
    auto loc_actors = UD::ActorSlotManager::GetSingleton()->GetValidActors();
    for (auto&& it :_actors)
    {
        //LOG("OrgasmManager::Update({}) - Updating actor {}",a_delta,it.first->GetName())
        RE::Actor* loc_actor = RE::Actor::LookupByHandle(it.first).get();
        OrgasmActorData* loc_actororgasm = &it.second;
        if (loc_actororgasm != nullptr && loc_actor != nullptr && (std::find(loc_actors.begin(),loc_actors.end(), it.first) != loc_actors.end()))
        {
            loc_actororgasm->SetActor(loc_actor);
            ////create thread for every actor
            loc_threads.push_back(std::thread(&OrgasmActorData::Update,loc_actororgasm,a_delta));
        }
        else
        {
            //error
        }
    }

    

    for (auto&& it : loc_threads) it.join();

    //LOG("OrgasmManager::Update({}) - Done",a_delta)
}

bool ORS::OrgasmManager::AddOrgasmChange(RE::Actor* a_actor, std::string a_key, OrgasmMod a_mod, EroZone a_erozones, float a_orgasmRate, float a_orgasmRateMult, float a_orgasmForcing, float a_orgasmCapacity, float a_orgasmResisten, float a_orgasmResistenceMult)
{
    if (a_actor == nullptr) return false;
    //LOG("OrgasmManager::AddOrgasmChange({},{},{},{},{})",a_actor->GetName(),a_key,a_mod,a_erozones,a_orgasmRate)
    UniqueLock lock(_lock);

    GETORGCHANGEANDVALIDATE(loc_oc,a_actor)
    //UniqueLock oclock(loc_oc.lock);
    return loc_oc.AddOrgasmChange(a_key,a_mod,a_erozones,a_orgasmRate,a_orgasmRateMult,a_orgasmForcing,a_orgasmCapacity,a_orgasmResisten,a_orgasmResistenceMult);
}

bool ORS::OrgasmManager::AddOrgasmChange(RE::Actor* a_actor, std::string a_key, OrgasmMod a_mod, uint32_t a_erozones, float a_orgasmRate, float a_orgasmRateMult, float a_orgasmForcing, float a_orgasmCapacity, float a_orgasmResisten, float a_orgasmResistenceMult)
{
    if (a_actor == nullptr) return false;
    //LOG("OrgasmManager::AddOrgasmChange({},{},{},{},{})",a_actor->GetName(),a_key,a_mod,a_erozones,a_orgasmRate)
    UniqueLock lock(_lock);

    GETORGCHANGEANDVALIDATE(loc_oc,a_actor)
    //UniqueLock oclock(loc_oc.lock);
    return loc_oc.AddOrgasmChange(a_key,a_mod,(EroZone)a_erozones,a_orgasmRate,a_orgasmRateMult,a_orgasmForcing,a_orgasmCapacity,a_orgasmResisten,a_orgasmResistenceMult);
}

bool ORS::OrgasmManager::RemoveOrgasmChange(RE::Actor* a_actor, std::string a_key)
{
    if (a_actor == nullptr) return false;
    //LOG("OrgasmManager::RemoveOrgasmChange({},{})",a_actor->GetName(),a_key)
    UniqueLock lock(_lock);

    GETORGCHANGEANDVALIDATE(loc_oc,a_actor)
    //UniqueLock oclock(loc_oc.lock);
    return loc_oc.RemoveOrgasmChange(a_key);
}

bool ORS::OrgasmManager::UpdateOrgasmChangeVar(RE::Actor* a_actor, std::string a_key, OrgasmVariable a_variable, float a_value, OrgasmUpdateType a_mod)
{
    if (a_actor == nullptr) return false;
    //LOG("OrgasmManager::UpdateOrgasmChangeVar({},{},{},{},{})",a_actor->GetName(),a_key,a_variable,a_value,a_mod)
    UniqueLock lock(_lock);

    GETORGCHANGEANDVALIDATE(loc_oc,a_actor)
    //UniqueLock oclock(loc_oc.lock);
    return loc_oc.UpdateOrgasmChangeVar(a_key,a_variable,a_value,a_mod);
}

float ORS::OrgasmManager::GetOrgasmChangeVar(RE::Actor* a_actor, std::string a_key, OrgasmVariable a_variable)
{
    if (a_actor == nullptr) return 0.0f;
    //LOG("OrgasmManager::GetOrgasmChangeVar({},{},{})",a_actor->GetName(),a_key,a_variable)
    UniqueLock lock(_lock);

    GETORGCHANGEANDVALIDATE(loc_oc,a_actor)
    //UniqueLock oclock(loc_oc.lock);
    return loc_oc.GetOrgasmChangeVar(a_key,a_variable);
}

bool ORS::OrgasmManager::HaveOrgasmChange(RE::Actor* a_actor, std::string a_key)
{
    if (a_actor == nullptr) return false;
    //LOG("OrgasmManager::HaveOrgasmChange({},{})",a_actor->GetName(),a_key)
    UniqueLock lock(_lock);

    GETORGCHANGEANDVALIDATE(loc_oc,a_actor)
    //UniqueLock oclock(loc_oc.lock);
    return loc_oc.HaveOrgasmChange(a_key);
}

float ORS::OrgasmManager::GetOrgasmProgress(RE::Actor* a_actor, int a_mod)
{
    if (a_actor == nullptr) return 0.0f;
    //LOG("OrgasmManager::GetOrgasmProgress({},{})",a_actor->GetName(),a_mod)
    UniqueLock lock(_lock);

    GETORGCHANGEANDVALIDATE(loc_oc,a_actor)
    //UniqueLock oclock(loc_oc.lock);
    return loc_oc.GetOrgasmProgress(a_mod);
}

void ORS::OrgasmManager::ResetOrgasmProgress(RE::Actor* a_actor)
{
    if (a_actor == nullptr) return;
    UniqueLock lock(_lock);

    GETORGCHANGEANDVALIDATE(loc_oc,a_actor)
    //UniqueLock oclock(loc_oc.lock);
    loc_oc.ResetOrgasmProgress();
}

float ORS::OrgasmManager::GetOrgasmVariable(RE::Actor* a_actor, OrgasmVariable a_variable)
{
    if (a_actor == nullptr) return 0.0f;
    //LOG("OrgasmManager::GetOrgasmVariable({},{})",a_actor->GetName(),a_variable)
    UniqueLock lock(_lock);
    
    GETORGCHANGEANDVALIDATE(loc_oc,a_actor)
    //UniqueLock oclock(loc_oc.lock);
    return loc_oc.GetOrgasmVariable(a_variable);
}

float ORS::OrgasmManager::GetAntiOrgasmRate(RE::Actor* a_actor)
{
    if (a_actor == nullptr) return 0.0f;
    //LOG("OrgasmManager::GetAntiOrgasmRate({})",a_actor->GetName())
    UniqueLock lock(_lock);

    GETORGCHANGEANDVALIDATE(loc_oc,a_actor)
    //UniqueLock oclock(loc_oc.lock);
    return loc_oc.GetAntiOrgasmRate();
}

void ORS::OrgasmManager::LinkActorToMeter(RE::Actor* a_actor, std::string a_path, MeterWidgetType a_type, int a_id)
{
    if (a_actor == nullptr) return;
    //LOG("OrgasmManager::LinkActorToMeter({})",a_actor->GetName(),a_path,a_type,a_id)
    UniqueLock lock(_lock);

    GETORGCHANGEANDVALIDATE(loc_oc,a_actor)
    //UniqueLock oclock(loc_oc.lock);
    return loc_oc.LinkActorToMeter(a_path,a_type,a_id);
}

void ORS::OrgasmManager::UnlinkActorFromMeter(RE::Actor* a_actor)
{
    if (a_actor == nullptr) return;
    //LOG("OrgasmManager::UnlinkActorFromMeter({})",a_actor->GetName())
    UniqueLock lock(_lock);

    GETORGCHANGEANDVALIDATE(loc_oc,a_actor)
    //UniqueLock oclock(loc_oc.lock);
    return loc_oc.UnlinkActorFromMeter();
}

std::string ORS::OrgasmManager::MakeUniqueKey(RE::Actor* a_actor,std::string a_base)
{
    if (a_actor == nullptr) return "ERROR";
    //LOG("OrgasmManager::MakeUniqueKey({},{})",a_actor->GetName(),a_base)
    UniqueLock lock(_lock);

    GETORGCHANGEANDVALIDATE(loc_oc,a_actor)
    //UniqueLock oclock(loc_oc.lock);
    return loc_oc.MakeUniqueKey(a_base);
}

std::vector<std::string> ORS::OrgasmManager::GetAllOrgasmChanges(RE::Actor* a_actor)
{
    if (a_actor == nullptr) return std::vector<std::string>();
    //LOG("OrgasmManager::UnlinkActorFromMeter({})",a_actor->GetName())
    UniqueLock lock(_lock);

    GETORGCHANGEANDVALIDATE(loc_oc,a_actor)
    //UniqueLock oclock(loc_oc.lock);
    return loc_oc.GetAllOrgasmChanges();
}

int ORS::OrgasmManager::RemoveAllOrgasmChanges(RE::Actor* a_actor)
{
    if (a_actor == nullptr) return 0;
    //LOG("OrgasmManager::UnlinkActorFromMeter({})",a_actor->GetName())
    UniqueLock lock(_lock);

    GETORGCHANGEANDVALIDATE(loc_oc,a_actor)
    //UniqueLock oclock(loc_oc.lock);
    return loc_oc.RemoveAllOrgasmChanges();
}

bool ORS::OrgasmManager::IsOrgasming(RE::Actor* a_actor)
{
    if (a_actor == nullptr) return 0;
    //LOG("OrgasmManager::IsOrgasming({})",a_actor->GetName())
    UniqueLock lock(_lock);

    GETORGCHANGEANDVALIDATE(loc_oc,a_actor)
    //UniqueLock oclock(loc_oc.lock);
    return loc_oc.IsOrgasming();
}

int ORS::OrgasmManager::GetOrgasmingCount(RE::Actor* a_actor)
{
    if (a_actor == nullptr) return 0;
    //LOG("OrgasmManager::GetOrgasmingCount({})",a_actor->GetName())
    UniqueLock lock(_lock);

    GETORGCHANGEANDVALIDATE(loc_oc,a_actor)
    //UniqueLock oclock(loc_oc.lock);
    return loc_oc.GetOrgasmingCount();
}

void ORS::OrgasmManager::Orgasm(RE::Actor* a_actor)
{
    if (a_actor == nullptr) return;
    //LOG("OrgasmManager::GetOrgasmingCount({})",a_actor->GetName())
    UniqueLock lock(_lock);

    GETORGCHANGEANDVALIDATE(loc_oc,a_actor)
    //UniqueLock oclock(loc_oc.lock);
    loc_oc.Orgasm();
}

std::string ORS::OrgasmManager::GetHornyStatus(RE::Actor* a_actor)
{
    if (a_actor == nullptr) return "ERROR";
    //LOG("OrgasmManager::GetHornyStatus({})",a_actor->GetName())
    UniqueLock lock(_lock);

    GETORGCHANGEANDVALIDATE(loc_oc,a_actor)
    //UniqueLock oclock(loc_oc.lock);
    return loc_oc.GetHornyStatus();
}

void ORS::OrgasmManager::RegisterPapyrusFunctions(RE::BSScript::IVirtualMachine *vm)
{
    #define REGISTERPAPYRUSFUNC(name,unhook) vm->RegisterFunction(#name, "OrgasmSystem", ORS::name,unhook);
    // ----
    REGISTERPAPYRUSFUNC(AddOrgasmChange,true)
    REGISTERPAPYRUSFUNC(RemoveOrgasmChange,true)
    REGISTERPAPYRUSFUNC(UpdateOrgasmChangeVar,true)
    REGISTERPAPYRUSFUNC(GetOrgasmChangeVar,true)
    REGISTERPAPYRUSFUNC(HaveOrgasmChange,true)
    REGISTERPAPYRUSFUNC(GetOrgasmProgress,true)
    REGISTERPAPYRUSFUNC(GetOrgasmVariable,true)  
    REGISTERPAPYRUSFUNC(GetAntiOrgasmRate,true)
    REGISTERPAPYRUSFUNC(ResetOrgasmProgress,true)
    REGISTERPAPYRUSFUNC(LinkActorToMeter,true)
    REGISTERPAPYRUSFUNC(UnlinkActorFromMeter,true)
    REGISTERPAPYRUSFUNC(MakeUniqueKey,true)
    REGISTERPAPYRUSFUNC(GetAllOrgasmChanges,true)
    REGISTERPAPYRUSFUNC(RemoveAllOrgasmChanges,true)
    REGISTERPAPYRUSFUNC(IsOrgasming,true)
    REGISTERPAPYRUSFUNC(GetOrgasmingCount,true)
    REGISTERPAPYRUSFUNC(ForceOrgasm,true)
    REGISTERPAPYRUSFUNC(GetHornyStatus,true)
    // ----
    #undef REGISTERPAPYRUSFUNC

    OrgasmEvents::GetSingleton()->RegisterPapyrus(vm);
}

void ORS::OrgasmManager::OnGameLoaded(SKSE::SerializationInterface* serde)
{
    uint32_t loc_type;
    uint32_t loc_size;
    uint32_t loc_version;

    if (serde == nullptr) return;

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
                    LOG("Actor ID {:X} could not be found after loading the save.", loc_actorFormID)
                    continue;
                }

                RE::Actor* loc_actor = RE::TESForm::LookupByID<RE::Actor>(loc_newActorFormID);

                if (loc_actor == nullptr)
                {
                    WARN("ERROR: Null actor ({:08X} -> {:08X}) found in cosave -> skipping",loc_actorFormID,loc_newActorFormID)

                    //read data in same way even if actor is null, so correct data are read next time
                    OrgasmActorData loc_data;
                    loc_data.OnGameLoaded(serde);

                    continue;
                }

                UniqueLock lock(_lock);

                LOG("Loaded actor {} from save",loc_actor->GetName())
                auto loc_handle = loc_actor->GetHandle().native_handle();
                _actors[loc_handle] = OrgasmActorData();
                _actors[loc_handle].SetActor(loc_actor);
                _actors[loc_handle].UpdatePosition();
                _actors[loc_handle].OnGameLoaded(serde);
            }
        }
    }

}

void ORS::OrgasmManager::OnGameSaved(SKSE::SerializationInterface* serde)
{
    UniqueLock lock(_lock);

    if (!serde->OpenRecord(OrgasmSerData, 0)) {
        return;
    }

    const size_t loc_actornum = _actors.size();
    serde->WriteRecordData(&loc_actornum,sizeof(size_t)); //first number of actors

    //now iterate thru all actors
    for (auto&& it : _actors)
    {
        RE::Actor* loc_actor = RE::Actor::LookupByHandle(it.first).get();
        OrgasmActorData loc_od = it.second;
        RE::FormID loc_formid = loc_actor->GetFormID();
        serde->WriteRecordData(&loc_formid,sizeof(RE::FormID));
        LOG("Saving actor {}",loc_actor->GetName())
        loc_od.OnGameSaved(serde);
    }

}

void ORS::OrgasmManager::OnRevert(SKSE::SerializationInterface* serde)
{
    UniqueLock lock(_lock);

    for (auto&& it : _actors)
    {
        it.second.OnRevert(serde);
    }

    _actors.clear();
}