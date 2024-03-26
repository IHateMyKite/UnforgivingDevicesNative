#include <UD_Serialization.h>
#include <UD_Utility.h>
#include <UD_Config.h>
#include <UD_PapyrusDelegate.h>
#include <OrgasmSystem/OrgasmManager.h>

namespace UD
{
    void OnGameLoaded(SKSE::SerializationInterface* serde)
    {
        LoadMode loc_LoadMode = (LoadMode)UD::Config::GetSingleton()->GetVariable<int>("General.iLoadMode",0);

        switch(loc_LoadMode)
        {
            case mDefault:
                LOG("Loading data from Cosave")
                ORS::OrgasmManager::GetSingleton()->OnGameLoaded(serde);
                break;
            case mSafe:
                LOG("!!!Safe load enabled - Not loading data from Cosave")
                // do nothing
                break;
        }

        PapyrusDelegate::GetSingleton()->Reload();
    }
    void OnGameSaved(SKSE::SerializationInterface* serde)
    {
        LOG("Saving data to Cosave")
        ORS::OrgasmManager::GetSingleton()->OnGameSaved(serde);
    }
    void OnRevert(SKSE::SerializationInterface* serde)
    {
        LOG("Reverting Cosave data")
        ORS::OrgasmManager::GetSingleton()->OnRevert(serde);
    }
}