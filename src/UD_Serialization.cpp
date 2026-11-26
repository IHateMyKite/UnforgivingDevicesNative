#include <UD_Serialization.h>
#include <UD_Utility.h>
#include <UD_Config.h>
#include <UD_PapyrusDelegate.h>
#include <UD_Animation.h>
#include <OrgasmSystem/OrgasmManager.h>
#include <UD_MinigameManager.h>
#include <UD_SaveManager.h>

namespace UD
{
    void OnGameLoaded(SKSE::SerializationInterface* serde)
    {
        DEBUG("OnGameLoaded called")
        LoadMode loc_LoadMode = (LoadMode)UD::Config::GetSingleton()->GetVariable<int>("General.iLoadMode",0);

        switch(loc_LoadMode)
        {
            case mDefault:
                LOG("Loading data from Cosave")

                uint32_t loc_type;
                uint32_t loc_size;
                uint32_t loc_version;

                while (serde->GetNextRecordInfo(loc_type, loc_version, loc_size))
                {
                    DEBUG("Reading record 0x{:08X} of size {}",loc_type,loc_size)
                    ORS::OrgasmManager::GetSingleton()->OnGameLoaded(serde,loc_type,loc_size,loc_version);
                    UD::SaveManager::GetSingleton()->OnGameLoaded(serde,loc_type,loc_size,loc_version);
                    UD::MinigameManager::GetSingleton()->OnGameLoaded(serde,loc_type,loc_size,loc_version);
                }
                break;
            case mSafe:
                LOG("!!!Safe load enabled - Not loading data from Cosave")
                // do nothing
                break;
        }

        PapyrusDelegate::GetSingleton()->Reload();
        AnimationManager::GetSingleton()->Reload();
        
    }
    void OnGameSaved(SKSE::SerializationInterface* serde)
    {
        DEBUG("Saving data to Cosave")
        ORS::OrgasmManager::GetSingleton()->OnGameSaved(serde);
        UD::SaveManager::GetSingleton()->OnGameSaved(serde);
        UD::MinigameManager::GetSingleton()->OnGameSaved(serde);
    }
    void OnRevert(SKSE::SerializationInterface* serde)
    {
        DEBUG("Reverting Cosave data")
        ORS::OrgasmManager::GetSingleton()->OnRevert(serde);
        UD::SaveManager::GetSingleton()->OnRevert(serde);
        UD::MinigameManager::GetSingleton()->OnRevert(serde);
    }
}