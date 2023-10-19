#include <OrgasmSystem/OrgasmManager.h>

SINGLETONBODY(ORS::OrgasmManager)

namespace ORS
{
    inline const auto OrgasmSerData = _byteswap_ulong('ORSD');
    ModifyArousal OSLAModifyArousal;
}

void ORS::OrgasmManager::Setup()
{
    if (!_installed)
    {
        //g_ArousalFaction = reinterpret_cast<RE::TESFaction*>(RE::TESDataHandler::GetSingleton()->LookupForm(0x1579C0,"UnforgivingDevices.esp"));
        HINSTANCE dllHandle = LoadLibrary(TEXT("OSLAroused.dll"));
        if (dllHandle != NULL)
        {
            FARPROC pModifyArousal = GetProcAddress(HMODULE (dllHandle),"ModifyArousalExt");
            OSLAModifyArousal = ModifyArousal(pModifyArousal);
            UDSKSELOG("OrgasmManager::Setup() - Modify arousal imported")
            //FreeLibrary(dllHandle);
        }
        else
        {
            UDSKSELOG("OrgasmManager::Setup() - ERROR: Cant find OSLAroused.dll!!")
        }
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
    //UDSKSELOG("OrgasmManager::AddOrgasmChange({},{},{},{},{})",a_actor->GetName(),a_key,a_mod,a_erozones,a_orgasmRate)
    std::unique_lock lock(_lock);
    return _actors[a_actor].AddOrgasmChange(a_key,a_mod,a_erozones,a_orgasmRate,a_orgasmRateMult,a_orgasmForcing,a_orgasmCapacity,a_orgasmResisten,a_orgasmResistenceMult);
}

bool ORS::OrgasmManager::AddOrgasmChange(RE::Actor* a_actor, std::string a_key, OrgasmMod a_mod, uint32_t a_erozones, float a_orgasmRate, float a_orgasmRateMult, float a_orgasmForcing, float a_orgasmCapacity, float a_orgasmResisten, float a_orgasmResistenceMult)
{
    if (a_actor == nullptr) return false;
    //UDSKSELOG("OrgasmManager::AddOrgasmChange({},{},{},{},{})",a_actor->GetName(),a_key,a_mod,a_erozones,a_orgasmRate)
    std::unique_lock lock(_lock);

    GETORGCHANGEANDVALIDATE(loc_oc,a_actor)

    return loc_oc.AddOrgasmChange(a_key,a_mod,(EroZone)a_erozones,a_orgasmRate,a_orgasmRateMult,a_orgasmForcing,a_orgasmCapacity,a_orgasmResisten,a_orgasmResistenceMult);
}

bool ORS::OrgasmManager::RemoveOrgasmChange(RE::Actor* a_actor, std::string a_key)
{
    if (a_actor == nullptr) return false;
    //UDSKSELOG("OrgasmManager::RemoveOrgasmChange({},{})",a_actor->GetName(),a_key)
    std::unique_lock lock(_lock);

    GETORGCHANGEANDVALIDATE(loc_oc,a_actor)

    return loc_oc.RemoveOrgasmChange(a_key);
}

bool ORS::OrgasmManager::UpdateOrgasmChangeVar(RE::Actor* a_actor, std::string a_key, OrgasmVariable a_variable, float a_value, OrgasmUpdateType a_mod)
{
    if (a_actor == nullptr) return false;
    //UDSKSELOG("OrgasmManager::UpdateOrgasmChangeVar({},{},{},{},{})",a_actor->GetName(),a_key,a_variable,a_value,a_mod)
    std::unique_lock lock(_lock);

    GETORGCHANGEANDVALIDATE(loc_oc,a_actor)

    return loc_oc.UpdateOrgasmChangeVar(a_key,a_variable,a_value,a_mod);
}

float ORS::OrgasmManager::GetOrgasmChangeVar(RE::Actor* a_actor, std::string a_key, OrgasmVariable a_variable)
{
    if (a_actor == nullptr) return 0.0f;
    //UDSKSELOG("OrgasmManager::GetOrgasmChangeVar({},{},{})",a_actor->GetName(),a_key,a_variable)
    std::unique_lock lock(_lock);
    
    GETORGCHANGEANDVALIDATE(loc_oc,a_actor)

    return loc_oc.GetOrgasmChangeVar(a_key,a_variable);
}

bool ORS::OrgasmManager::HaveOrgasmChange(RE::Actor* a_actor, std::string a_key)
{
    if (a_actor == nullptr) return false;
    //UDSKSELOG("OrgasmManager::HaveOrgasmChange({},{})",a_actor->GetName(),a_key)
    std::unique_lock lock(_lock);

    GETORGCHANGEANDVALIDATE(loc_oc,a_actor)

    return loc_oc.HaveOrgasmChange(a_key);
}

float ORS::OrgasmManager::GetOrgasmProgress(RE::Actor* a_actor, int a_mod)
{
    if (a_actor == nullptr) return 0.0f;
    //UDSKSELOG("OrgasmManager::GetOrgasmProgress({},{})",a_actor->GetName(),a_mod)
    std::unique_lock lock(_lock);

    GETORGCHANGEANDVALIDATE(loc_oc,a_actor)

    return loc_oc.GetOrgasmProgress(a_mod);
}

void ORS::OrgasmManager::ResetOrgasmProgress(RE::Actor* a_actor)
{
    if (a_actor == nullptr) return;
    std::unique_lock lock(_lock);

    GETORGCHANGEANDVALIDATE(loc_oc,a_actor)

    loc_oc.ResetOrgasmProgress();
}

float ORS::OrgasmManager::GetOrgasmVariable(RE::Actor* a_actor, OrgasmVariable a_variable)
{
    if (a_actor == nullptr) return 0.0f;
    //UDSKSELOG("OrgasmManager::GetOrgasmVariable({},{})",a_actor->GetName(),a_variable)
    std::unique_lock lock(_lock);
    
    GETORGCHANGEANDVALIDATE(loc_oc,a_actor)

    return loc_oc.GetOrgasmVariable(a_variable);
}

float ORS::OrgasmManager::GetAntiOrgasmRate(RE::Actor* a_actor)
{
    if (a_actor == nullptr) return 0.0f;
    //UDSKSELOG("OrgasmManager::GetAntiOrgasmRate({})",a_actor->GetName())
    std::unique_lock lock(_lock);

    GETORGCHANGEANDVALIDATE(loc_oc,a_actor)

    return loc_oc.GetAntiOrgasmRate();
}

void ORS::OrgasmManager::LinkActorToMeter(RE::Actor* a_actor, std::string a_path, MeterWidgetType a_type, int a_id)
{
    if (a_actor == nullptr) return;
    //UDSKSELOG("OrgasmManager::LinkActorToMeter({})",a_actor->GetName(),a_path,a_type,a_id)
    std::unique_lock lock(_lock);

    GETORGCHANGEANDVALIDATE(loc_oc,a_actor)

    return loc_oc.LinkActorToMeter(a_path,a_type,a_id);
}

void ORS::OrgasmManager::UnlinkActorFromMeter(RE::Actor* a_actor)
{
    if (a_actor == nullptr) return;
    //UDSKSELOG("OrgasmManager::UnlinkActorFromMeter({})",a_actor->GetName())
    std::unique_lock lock(_lock);

    GETORGCHANGEANDVALIDATE(loc_oc,a_actor)

    return loc_oc.UnlinkActorFromMeter();
}

std::string ORS::OrgasmManager::MakeUniqueKey(RE::Actor* a_actor,std::string a_base)
{
    if (a_actor == nullptr) return "ERROR";
    //UDSKSELOG("OrgasmManager::MakeUniqueKey({},{})",a_actor->GetName(),a_base)
    std::unique_lock lock(_lock);

    GETORGCHANGEANDVALIDATE(loc_oc,a_actor)

    return loc_oc.MakeUniqueKey(a_base);
}

std::vector<std::string> ORS::OrgasmManager::GetAllOrgasmChanges(RE::Actor* a_actor)
{
    if (a_actor == nullptr) return std::vector<std::string>();
    //UDSKSELOG("OrgasmManager::UnlinkActorFromMeter({})",a_actor->GetName())
    std::unique_lock lock(_lock);

    GETORGCHANGEANDVALIDATE(loc_oc,a_actor)

    return loc_oc.GetAllOrgasmChanges();
}

int ORS::OrgasmManager::RemoveAllOrgasmChanges(RE::Actor* a_actor)
{
    if (a_actor == nullptr) return 0;
    //UDSKSELOG("OrgasmManager::UnlinkActorFromMeter({})",a_actor->GetName())
    std::unique_lock lock(_lock);

    GETORGCHANGEANDVALIDATE(loc_oc,a_actor)

    return loc_oc.RemoveAllOrgasmChanges();
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
    // ----
    #undef REGISTERPAPYRUSFUNC
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
                    UDSKSELOG("Actor ID {:X} could not be found after loading the save.", loc_actorFormID)
                    continue;
                }

                RE::Actor* loc_actor = RE::TESForm::LookupByID<RE::Actor>(loc_newActorFormID);

                if (loc_actor == nullptr)
                {
                    UDSKSELOG("ERROR: Null actor ({:08X} -> {:08X}) found in cosave -> skipping",loc_actorFormID,loc_newActorFormID)

                    //read data in same way even if actor is null, so correct data are rad next time
                    OrgasmActorData loc_data;
                    loc_data.OnGameLoaded(serde);

                    continue;
                }

                UDSKSELOG("Loaded actor {} from save",loc_actor->GetName())
                _actors[loc_actor] = OrgasmActorData();
                _actors[loc_actor].SetActor(loc_actor);
                _actors[loc_actor].UpdatePosition();
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