#include "UD_FastTravelManager.h"
#include "UD_Utility.h"
#include "UD_Config.h"

SINGLETONBODY(UD::FastTravelManager)

void UD::FastTravelManager::Setup()
{
    if (!_init)
    {
        //hook function
         HookVirtualMethod<decltype(FastTravelConfirmCallback_Run_Patched)>(RE::FastTravelConfirmCallback::VTABLE,01,01,reinterpret_cast<uintptr_t>(FastTravelConfirmCallback_Run_Patched),FastTravelConfirmCallback_Run);
        _init = true;
        DEBUG("Setup() - Initiated")
    }
}

void UD::FastTravelManager::FastTravelConfirmCallback_Run_Patched(RE::FastTravelConfirmCallback* a_this, RE::IMessageBoxCallback::Message a_msg)
{
    LOG("FastTravelConfirmCallback_Run_Patched called = Msg = {}",(int)a_msg)

    // Read config variables from ini file
    const bool loc_hb       = Config::GetSingleton()->GetVariable<bool>("FastTravel.bHandRestrain",false);
    const bool loc_boots    = Config::GetSingleton()->GetVariable<bool>("FastTravel.bBoots",false);
    const bool loc_blind    = Config::GetSingleton()->GetVariable<bool>("FastTravel.bBlindfold",false);
    const bool loc_hobble   = Config::GetSingleton()->GetVariable<bool>("FastTravel.bHobble",true);
    const auto loc_kwds     = Config::GetSingleton()->GetArrayText("FastTravel.asAdditionalKeywords",false);

    auto        loc_player  = RE::PlayerCharacter::GetSingleton();
    static auto loc_utility = Utility::GetSingleton();

    bool        loc_disable = false;
    std::string loc_msg     = "ERROR";

    if (loc_hb && loc_utility->WornHasKeyword(loc_player,"zad_DeviousHeavyBondage"))
    {
        loc_disable = true;
        loc_msg = "You can't fast travel while being bound helpless!";
    }
    else if (loc_boots && loc_utility->WornHasKeyword(loc_player,"zad_DeviousBoots"))
    {
        loc_disable = true;
        loc_msg = "You can't fast travel while being locked in restrictive boots!";
    }
    else if (loc_blind && loc_utility->WornHasKeyword(loc_player,"zad_DeviousBlindfold"))
    {
        loc_disable = true;
        loc_msg = "You can't fast travel while being blind!";
    }
    else if (loc_hobble && loc_utility->WornHasKeyword(loc_player,"zad_DeviousHobbleSkirt"))
    {
        loc_disable = true;
        loc_msg = "You can't fast travel while being locked in hobble skirt!";
    }
    else
    {
        //additional keywords
        for (auto&& it : loc_kwds)
        {
            if (loc_utility->WornHasKeyword(loc_player,it))
            {
                loc_disable = true;
                loc_msg = "You can't fast travel in your current state!";
                break;
            }
        }
    }

    //Message values
    // 0 = ???
    // 1 = Yes
    // 2 = No
    // 3 = Place a marker

    //If player selected Yes, change it to No :)
    if (loc_disable && (int)a_msg == 1)
    {
        a_msg = (RE::IMessageBoxCallback::Message)2;
        RE::DebugNotification(loc_msg.c_str());
    }
    FastTravelManager::GetSingleton()->FastTravelConfirmCallback_Run(a_this,a_msg);
}
