#include <UD_ControlManager.h>


SINGLETONBODY(UD::KeyEventSink)
SINGLETONBODY(UD::ControlManager)

void UD::ControlManager::Setup()
{
    if (!_installed)
    {
        _installed = true;

        RE::TESDataHandler* loc_datahandler = RE::TESDataHandler::GetSingleton();

        _boundkeywords.push_back(static_cast<RE::BGSKeyword*>(loc_datahandler->LookupForm(0x05226C,"Devious Devices - Integration.esm")));

        _minigamefaction = static_cast<RE::TESFaction*>(loc_datahandler->LookupForm(0x150DA3,"UnforgivingDevices.esp"));
        _animationfaction = static_cast<RE::TESFaction*>(loc_datahandler->LookupForm(0x029567,"Devious Devices - Integration.esm"));

        //init control map
        auto loc_control = RE::ControlMap::GetSingleton()->controlMap[RE::ControlMap::InputContextID::kGameplay]->deviceMappings;
        //UDSKSELOG("Number of mappings = {}",(*loc_control).size())
        for (int i = 0; i <= RE::INPUT_DEVICES::kVirtualKeyboard; i++)
        {
            //UDSKSELOG("Checking mapping of size {}",loc_control[i].size())
            for (auto&& it : loc_control[i]) 
            {
                //UDSKSELOG("Checking {}",it.eventID)
                {
                    const auto loc_foundit = std::find_if(_hardcoreids.begin(),_hardcoreids.end(),[it](const RE::BSFixedString& a_id)
                    {
                        if (it.eventID == a_id)return true;
                        return false;
                    
                    });

                    const bool loc_found = (loc_foundit != _hardcoreids.end());
                    if (loc_found)
                    {
                        _hardcorecontrols.push_back({&it,it,(RE::INPUT_DEVICE)i});
                        UDSKSELOG("Hardcore control added - {} , {} , {} , {} ",it.eventID,it.inputKey,it.linked,it.remappable)
                    } 
                }

                {
                    const auto loc_foundit = std::find_if(_minigameids.begin(),_minigameids.end(),[it](const RE::BSFixedString& a_id)
                    {
                        if (it.eventID == a_id)return true;
                        return false;
                    
                    });

                    const bool loc_found = (loc_foundit != _minigameids.end());
                    if (loc_found)
                    {
                        _minigamecontrols.push_back({&it,it,(RE::INPUT_DEVICE)i});
                        UDSKSELOG("Minigame control added - {} , {} , {} , {} ",it.eventID,it.inputKey,it.linked,it.remappable)
                    } 
                }

            }
        }

    }
}

void UD::ControlManager::UpdateControl()
{
    //check if player is in minigame
    if (PlayerInMinigame() || PlayerInZadAnimation())
    {
        if (!_ControlsDisabled)
        {
            _ControlsDisabled = true;
            SKSE::GetTaskInterface()->AddTask([&]()
            {
                for (auto&& it : _minigamecontrols) 
                {
                    //UDSKSELOG("Disabling control for {} on key {}",it.ptr->eventID,it.ptr->inputKey)
                    if (it.ptr->inputKey != static_cast<uint16_t>(-1) && (it.ptr->inputKey != it.originalval.inputKey))
                    {
                        //update original value
                        it.originalval = *it.ptr;
                    }
                    it.ptr->inputKey = static_cast<uint16_t>(-1);
                }
            });
        }
    }
    else
    {
        if (_ControlsDisabled)
        {
            _ControlsDisabled = false;
            SKSE::GetTaskInterface()->AddTask([&]()
            {
                
                for (auto&& it : _minigamecontrols) 
                {
                    if (it.ptr->inputKey != static_cast<uint16_t>(-1) && (it.ptr->inputKey != it.originalval.inputKey))
                    {
                        //update original value
                        it.originalval = *it.ptr;
                    }
                    //UDSKSELOG("Restoring control for {} with key {} to {} with key {}",it.ptr->eventID,it.ptr->inputKey,it.originalval.eventID,it.originalval.inputKey)
                    *it.ptr = it.originalval;
                }
            });
        }
        else
        {
            if (_hardcoreMode)
            {
                if (PlayerIsBound())
                {
                    SKSE::GetTaskInterface()->AddTask([&]()
                    {
                        for (auto&& it : _hardcorecontrols) 
                        {
                            if (it.ptr->inputKey != static_cast<uint16_t>(-1) && (it.ptr->inputKey != it.originalval.inputKey))
                            {
                                //update original value
                                it.originalval = *it.ptr;
                            }
                            //UDSKSELOG("Disabling control for {} on key {}",it.ptr->eventID,it.ptr->inputKey)
                            it.ptr->inputKey = static_cast<uint16_t>(-1);
                        }
                    });
                }
                else
                {
                    SKSE::GetTaskInterface()->AddTask([&]()
                    {
                
                        for (auto&& it : _hardcorecontrols) 
                        {
                            if (it.ptr->inputKey != static_cast<uint16_t>(-1) && (it.ptr->inputKey != it.originalval.inputKey))
                            {
                                //update original value
                                it.originalval = *it.ptr;
                            }
                            //UDSKSELOG("Restoring control for {} with key {} to {} with key {}",it.ptr->eventID,it.ptr->inputKey,it.originalval.eventID,it.originalval.inputKey)
                            *it.ptr = it.originalval;
                        }
                    });
                }
            }
        }
    }

}

const std::vector<UD::ControlDisable>& UD::ControlManager::GetHardcoreControls() const
{
    return _hardcorecontrols;
}

void UD::ControlManager::SyncSetting(bool a_hardcoreMode)
{
    _hardcoreMode = a_hardcoreMode;
    if (_hardcoreMode)
    {
        UpdateControl();
    }
    else
    {
        SKSE::GetTaskInterface()->AddTask([&]()
        {
            for (auto&& it : _hardcorecontrols) *it.ptr = it.originalval;
        });
    }
    
}

bool UD::ControlManager::HardcoreMode() const
{
    return _hardcoreMode;
}

bool UD::ControlManager::PlayerIsBound() const
{
    RE::Actor* loc_player = RE::PlayerCharacter::GetSingleton();

    if (loc_player == nullptr) return false;

    //check normal hb device (like armbinder)
    const RE::TESObjectARMO* loc_hbdevice = loc_player->GetWornArmor(RE::BIPED_MODEL::BipedObjectSlot::kModChestPrimary);

    bool loc_res = loc_hbdevice && loc_hbdevice->HasKeywordInArray(_boundkeywords,false);

    if (!loc_res)
    {
        //check body hb (like straitjacket)
        const RE::TESObjectARMO* locbodydevice = loc_player->GetWornArmor(RE::BIPED_MODEL::BipedObjectSlot::kBody);

        loc_res = locbodydevice && locbodydevice->HasKeywordInArray(_boundkeywords,false);
    }
    return loc_res;
}

bool UD::ControlManager::PlayerInMinigame() const
{
    RE::Actor* loc_player = RE::PlayerCharacter::GetSingleton();

    if (loc_player == nullptr) return false;

    if (loc_player == nullptr || _minigamefaction == nullptr) return false;

    return loc_player->IsInFaction(_minigamefaction);
}

bool UD::ControlManager::PlayerInZadAnimation() const
{
    RE::Actor* loc_player = RE::PlayerCharacter::GetSingleton();

    if (loc_player == nullptr) return false;

    if (loc_player == nullptr || _animationfaction == nullptr) return false;

    return loc_player->IsInFaction(_animationfaction);
}

RE::BSEventNotifyControl UD::KeyEventSink::ProcessEvent(RE::InputEvent* const* eventPtr, RE::BSTEventSource<RE::InputEvent*>*)
{
    if (_Cooldown) return RE::BSEventNotifyControl::kContinue; //on cooldown - return
    if (!eventPtr) return RE::BSEventNotifyControl::kContinue;
    if (!ControlManager::GetSingleton()->HardcoreMode()) return RE::BSEventNotifyControl::kContinue; //no hardcore mode - return

    auto* event = *eventPtr;
    if (!event) return RE::BSEventNotifyControl::kContinue;

    if (event->GetEventType() == RE::INPUT_EVENT_TYPE::kButton) {
        const auto*       loc_buttonEvent = event->AsButtonEvent();
        const uint32_t    loc_dxScanCode  = loc_buttonEvent->GetIDCode();
        if (loc_buttonEvent->IsRepeating()) return RE::BSEventNotifyControl::kContinue;

        const RE::INPUT_DEVICE loc_Device = loc_buttonEvent->GetDevice();

        //UDSKSELOG("Key {} pressed",loc_dxScanCode)

        //check all saved controls for tween menu and compare
        auto loc_hc = ControlManager::GetSingleton()->GetHardcoreControls();

        const auto loc_it = std::find_if(loc_hc.begin(),loc_hc.end(),[loc_dxScanCode,loc_Device](const ControlDisable& a_control)
            {
                if ((loc_dxScanCode == a_control.originalval.inputKey) && (loc_Device == a_control.device)) return true;
                return false;
            }
        );

        if (loc_it != loc_hc.end())
        {
            if (ControlManager::GetSingleton()->PlayerIsBound() && !ControlManager::GetSingleton()->PlayerInMinigame() && !ControlManager::GetSingleton()->PlayerInZadAnimation())
            {
                SKSE::ModCallbackEvent modEvent{
                    "UD_HMTweenMenu",
                    "",
                    0.0f,
                    nullptr
                };

                UDSKSELOG("Sending player tween menu event")

                auto modCallback = SKSE::GetModCallbackEventSource();
                modCallback->SendEvent(&modEvent);

                std::thread([&]()
                {
                    _Cooldown = true;
                    std::this_thread::sleep_for(std::chrono::milliseconds(600));
                    _Cooldown = false;
                }).detach();
            }
        }
    }

    return RE::BSEventNotifyControl::kContinue;
}
