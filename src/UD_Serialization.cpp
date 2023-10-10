#include <UD_Serialization.h>

namespace UD
{
    void OnGameLoaded(SKSE::SerializationInterface* serde)
    {
        UDSKSELOG("Loading data from Cosave")
        ORS::OrgasmManager::GetSingleton()->OnGameLoaded(serde);
    }
    void OnGameSaved(SKSE::SerializationInterface* serde)
    {
        UDSKSELOG("¨Saving data to Cosave")
        ORS::OrgasmManager::GetSingleton()->OnGameSaved(serde);
    }
    void OnRevert(SKSE::SerializationInterface* serde)
    {
        UDSKSELOG("Reverting Cosave data")
        ORS::OrgasmManager::GetSingleton()->OnRevert(serde);
    }
}