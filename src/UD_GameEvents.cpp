#include <UD_GameEvents.h>

namespace UD
{
    inline void _OnGameLoad()
    {
        UD::ReloadLib();

        boost::property_tree::ptree loc_pt;
        bool loc_iniloaded = true;
        try
        {
            boost::property_tree::ini_parser::read_ini("Data\\skse\\plugins\\UDNative.ini", loc_pt);
        }
        catch( std::exception &ex )
        {
            // you either print it out or have a MessageBox pop up or hide it.
            UDSKSELOG("ERROR: {}",ex.what())
            loc_iniloaded = false;
        }

        PlayerStatus::GetSingleton()->Setup();
        MeterManager::RemoveAll();
        KeywordManager::Reload();
        InventoryHandler::Reload();
        ORS::OrgasmManager::GetSingleton()->Setup(loc_pt);
        ActorSlotManager::GetSingleton()->Setup();
        ControlManager::GetSingleton()->Setup(loc_pt);

        //remove effect in case that user reloaded the game without exit
        if (MinigameEffectManager::GetSingleton()->started) MinigameEffectManager::GetSingleton()->RemoveAll();

        UpdateManager::GetSingleton()->CreateUpdateThreads();
    }


    inline void _OnPostPostLoad()
    {
        UDSKSELOG("::_OnPostPostLoad called");
    }

    void OnMessageReceived(SKSE::MessagingInterface::Message* a_msg)
    {
        switch(a_msg->type)
        {
            case SKSE::MessagingInterface::kInputLoaded:
                RE::BSInputDeviceManager::GetSingleton()->AddEventSink(KeyEventSink::GetSingleton());
                break;
            case SKSE::MessagingInterface::kPostLoadGame:
            case SKSE::MessagingInterface::kNewGame:
                _OnGameLoad();
                break;
            case SKSE::MessagingInterface::kPostPostLoad:
                _OnPostPostLoad();
                break;
        }
    }
}