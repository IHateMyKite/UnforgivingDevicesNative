#include <UD_Serialization.h>

namespace UD
{
    void OnGameLoaded(SKSE::SerializationInterface* serde)
    {
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

        if (loc_iniloaded)
        {
            LoadMode loc_LoadMode = (LoadMode)loc_pt.get<int>("General.iLoadMode");

            switch(loc_LoadMode)
            {
                case mDefault:
                    UDSKSELOG("Loading data from Cosave")
                    ORS::OrgasmManager::GetSingleton()->OnGameLoaded(serde);
                    break;
                case mSafe:
                    UDSKSELOG("!!!Safe load enabled - Not loading data from Cosave")
                    // do nothing
                    break;
            }
        }
    }
    void OnGameSaved(SKSE::SerializationInterface* serde)
    {
        UDSKSELOG("Saving data to Cosave")
        ORS::OrgasmManager::GetSingleton()->OnGameSaved(serde);
    }
    void OnRevert(SKSE::SerializationInterface* serde)
    {
        UDSKSELOG("Reverting Cosave data")
        ORS::OrgasmManager::GetSingleton()->OnRevert(serde);
    }
}