#include <UD_GameEvents.h>
#include <UD_Lib.h>
#include <UD_MinigameEffect.h>
#include <UD_Utility.h>
#include <UD_UI.h>
#include <UD_Inventory.h>
#include <UD_ControlManager.h>
#include <UD_PapyrusDelegate.h>
#include <UD_ModEvents.h>
#include <UD_Animation.h>
#include <UD_Skill.h>
#include <UD_ActorSlotManager.h>
#include <UD_Materials.h>
#include <UD_Diagnosis.h>
#include <UD_Lockpick.h>
#include <UD_Updater.h>
#include <UD_PlayerStatus.h>
#include <UD_Keywords.h>
#include <UD_FastTravelManager.h>
#include <UD_Modifiers.h>
#include <UD_MessageBox.h>
#include <OrgasmSystem/OrgasmManager.h>
#include <UD_DDAPI.h>
#include <UD_Macros.h>
#include <UD_ModuleManager.h>

namespace UD
{
    inline void _OnGameLoad(bool a_newGame)
    {
        UD::ReloadLib();
        ModuleManager::GetSingleton()->Reload(a_newGame); // If new game, add delay, so update does not happen during char creation
        PapyrusDelegate::GetSingleton()->Setup();
        RandomGenerator::GetSingleton()->Setup();
        PlayerStatus::GetSingleton()->Setup();
        MeterManager::RemoveAll();
        KeywordManager::Reload();
        InventoryHandler::GetSingleton()->Reload();
        ORS::OrgasmManager::GetSingleton()->Setup();
        ActorSlotManager::GetSingleton()->Setup();
        ControlManager::GetSingleton()->Setup();
        MaterialManager::GetSingleton()->Setup();
        Diagnosis::GetSingleton()->Setup();
        LockpickManager::GetSingleton()->Setup();
        FastTravelManager::GetSingleton()->Setup();
        AnimationManager::GetSingleton()->Setup();
        ModifierManager::GetSingleton()->Setup();
        MessageboxManager::GetSingleton()->Setup();
        SkillManager::GetSingleton()->Setup();

        //remove effect in case that user reloaded the game without exit
        if (MinigameEffectManager::GetSingleton()->started) MinigameEffectManager::GetSingleton()->RemoveAll();

        UpdateManager::GetSingleton()->CreateUpdateThreads();
    }


    inline void _OnPostPostLoad()
    {
        //LOG("::_OnPostPostLoad called");
    }

    void OnMessageReceived(SKSE::MessagingInterface::Message* a_msg)
    {
        switch(a_msg->type)
        {
            case SKSE::MessagingInterface::kInputLoaded:
                RE::BSInputDeviceManager::GetSingleton()->AddEventSink(KeyEventSink::GetSingleton());
                break;
            case SKSE::MessagingInterface::kPostLoadGame:
                if (!DeviousDevicesAPI::LoadAPI()) ERROR("Could not load DD API!")
                _OnGameLoad(false);
                break;
            case SKSE::MessagingInterface::kNewGame:
                if (!DeviousDevicesAPI::LoadAPI()) ERROR("Could not load DD API!")
                _OnGameLoad(true);
                break;
            case SKSE::MessagingInterface::kPostPostLoad:
                _OnPostPostLoad();
                break;
        }
    }
}