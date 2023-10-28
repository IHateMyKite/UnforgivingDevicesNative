#include <UD_ControlManager.h>


SINGLETONBODY(UD::KeyEventSink)
SINGLETONBODY(UD::CameraEventSink)
SINGLETONBODY(UD::ControlManager)

void UD::ControlManager::Setup(const boost::property_tree::ptree& a_ptree)
{
    if (!_installed)
    {
        _installed = true;

        RE::TESDataHandler* loc_datahandler = RE::TESDataHandler::GetSingleton();

        _boundkeywords.push_back(static_cast<RE::BGSKeyword*>(loc_datahandler->LookupForm(0x05226C,"Devious Devices - Integration.esm")));

        _minigamefaction = static_cast<RE::TESFaction*>(loc_datahandler->LookupForm(0x150DA3,"UnforgivingDevices.esp"));
        _animationfaction = static_cast<RE::TESFaction*>(loc_datahandler->LookupForm(0x029567,"Devious Devices - Integration.esm"));

        //load memory
        _OriginalControls = new RE::BSTArray<RE::ControlMap::UserEventMapping>[4];
        SaveOriginalControls();

        boost::split(_hardcoreids,a_ptree.get<std::string>("Disabler.asHardcoreModeDisable"),boost::is_any_of(","));

        for (auto&& it :_hardcoreids) 
        {
            auto loc_first = it.find_first_not_of(' ');
            auto loc_last  = it.find_last_not_of(' ');
            it = it.substr(loc_first,loc_last - loc_first + 1);
        }

        UDSKSELOG("Hardcore disable config loaded. Number = {}",_hardcoreids.size())
        for (auto&& it : _hardcoreids) UDSKSELOG("{}",it)

        InitControlOverride(&_HardcoreControls,_hardcoreids);

        _disableids = _disablenomoveids; //copy disable which doesnt disable movement
        _disableids.insert(_disableids.begin(),{"Forward","Back","Strafe Right","Strafe Left"}); //add movement disable

        InitControlOverride(&_DisabledControls,_disableids);

        InitControlOverride(&_DisabledNoMoveControls,_disablenomoveids);

        SKSE::GetCameraEventSource()->AddEventSink(CameraEventSink::GetSingleton());

        UDSKSELOG("ControlManager installed")
        //DebugPrintControls(_HardcoreControls);
        //DebugPrintControls(_DisabledControls);
    }

    _DisableFreeCamera = a_ptree.get<bool>("Disabler.bDisableFreeCamera");
}

void UD::ControlManager::UpdateControl()
{
    //check if player is in minigame
    bool loc_status[3];

    CheckStatusSafe(loc_status);

    if ((loc_status[1] || loc_status[2]) && !RE::PlayerCamera::GetSingleton()->IsInFreeCameraMode())
    {
        if (!_ControlsDisabled)
        {
            _ControlsDisabled = true;
            SKSE::GetTaskInterface()->AddTask([this]()
            {
                DisableControls();
            });
        }
    }
    else
    {
        if (_ControlsDisabled)
        {
            _ControlsDisabled = false;
            SKSE::GetTaskInterface()->AddTask([this]()
            {
                ApplyControls(_OriginalControls);
            });
        }
        else
        {
            if (_hardcoreMode)
            {
                if (loc_status[0] && !_HardcoreModeApplied)
                {
                    _HardcoreModeApplied = true;
                    SKSE::GetTaskInterface()->AddTask([this]()
                    {
                        ApplyControls(_HardcoreControls);
                    });
                }
                else if (!loc_status[0] && _HardcoreModeApplied)
                {
                    _HardcoreModeApplied = false;
                    SKSE::GetTaskInterface()->AddTask([this]()
                    {
                        ApplyOriginalControls();
                    });
                }
            }
        }
    }
}

void UD::ControlManager::SyncSetting(bool a_hardcoreMode)
{
    _hardcoreMode = a_hardcoreMode;
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

void UD::ControlManager::CheckStatusSafe(bool* a_result)
{
    bool* loc_res = a_result;

    bool loc_fetchingstatus = true;

    //UDSKSELOG("ControlManager::CheckStatusSafe() - Sending check status task...")

    SKSE::GetTaskInterface()->AddTask([this,&loc_res,&loc_fetchingstatus]()
    {
        //UDSKSELOG("ControlManager::CheckStatusSafe() - Serial task received - checking status...")
        loc_res[0] = PlayerIsBound();
        loc_res[1] = PlayerInMinigame();
        loc_res[2] = PlayerInZadAnimation();
        loc_fetchingstatus = false;
    });

    while (loc_fetchingstatus)
    {
        //UDSKSELOG("ControlManager::CheckStatusSafe() - waiting...")
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }

    //UDSKSELOG("ControlManager::CheckStatusSafe() - Status checked - Bound = {} , Animating = {} , Minigame = {} , Free cam = {}",loc_res[0],loc_res[1],loc_res[2],RE::PlayerCamera::GetSingleton()->IsInFreeCameraMode())
}

void UD::ControlManager::DebugPrintControls(RE::BSTArray<RE::ControlMap::UserEventMapping>* a_controls)
{
    if (a_controls == nullptr) return;

    UDSKSELOG("==Printing controls , Size={}==",a_controls->size())
    for (int j = 0; j <= RE::INPUT_DEVICES::kVirtualKeyboard; j++)
    {
        UDSKSELOG("=INPUT_DEVICES = {:2}",j)
        for (auto&& it : a_controls[j]) 
        {
            UDSKSELOG("{:20} , {:6} , {:5} , {:5} , {:6} , {:08X} , {:3}",it.eventID,it.inputKey,it.linked,it.remappable,it.modifier,it.userEventGroupFlag.underlying(),it.indexInContext)
        }
    }
}

void UD::ControlManager::DebugPrintControls()
{
    UDSKSELOG("==Printing control==")
    for (int i = 0; i < RE::UserEvents::INPUT_CONTEXT_IDS::kTotal; i++)
    {
        UDSKSELOG("=INPUT_CONTEXT_IDS = {:2}",i)
        auto loc_control = RE::ControlMap::GetSingleton()->controlMap[i]->deviceMappings;
        for (int j = 0; j <= RE::INPUT_DEVICES::kVirtualKeyboard; j++)
        {
            UDSKSELOG("=INPUT_DEVICES = {:2}",j)
            for (auto&& it : loc_control[j]) 
            {
                UDSKSELOG("{:20} , {:6} , {:5} , {:5} , {:6} , {:08X} , {:3}",it.eventID,it.inputKey,it.linked,it.remappable,it.modifier,it.userEventGroupFlag.underlying(),it.indexInContext)
            }
        }
    }
}

bool UD::ControlManager::HardcoreButtonPressed(uint32_t a_dxkeycode, RE::INPUT_DEVICE a_device)
{
    for (int i = 0; i <= RE::INPUT_DEVICES::kVirtualKeyboard; i++)
    {
        for (auto&& it : _OriginalControls[i])
        {
            const auto loc_it = std::find_if(_hardcoreids.begin(),_hardcoreids.end(),[it,i,a_dxkeycode,a_device](const RE::BSFixedString& a_event)
                {
                    if ((a_device == i) && (it.inputKey == a_dxkeycode) && (it.eventID == a_event)) return true;
                    return false;
                }
            );
            if (loc_it != _hardcoreids.end()) return true;
        }
    }
    return false;
}

void UD::ControlManager::ApplyOriginalControls()
{
    ApplyControls(_OriginalControls);
}

void UD::ControlManager::DisableControls()
{
    ApplyControls(_DisabledControls);
}

void UD::ControlManager::DisableControlsFC()
{
    if (_DisableFreeCamera)
    {
        ApplyControls(_DisabledNoMoveControls);
    }
    else
    {
        ApplyOriginalControls();
    }
}

void UD::ControlManager::SaveOriginalControls()
{
    auto loc_control = RE::ControlMap::GetSingleton()->controlMap[RE::UserEvents::INPUT_CONTEXT_IDS::kGameplay]->deviceMappings;
    for (int i = 0; i <= RE::INPUT_DEVICES::kVirtualKeyboard; i++)
    {
        _OriginalControls[i] = loc_control[i];
    }
}

void UD::ControlManager::InitControlOverride(RE::BSTArray<RE::ControlMap::UserEventMapping>** a_controls,const std::vector<std::string>& a_filter)
{
    *a_controls = new RE::BSTArray<RE::ControlMap::UserEventMapping>[4];
    auto loc_control = RE::ControlMap::GetSingleton()->controlMap[RE::ControlMap::InputContextID::kGameplay]->deviceMappings;
    for (int i = 0; i <= RE::INPUT_DEVICES::kVirtualKeyboard; i++)
    {
        for (auto&& it : loc_control[i]) 
        {
            {
                const auto loc_foundit = std::find_if(a_filter.begin(),a_filter.end(),[it](const std::string& a_id)
                {
                    if (std::strcmp(it.eventID.c_str(),a_id.c_str()) == 0) return true;
                    return false;
                    
                });

                const bool loc_found = (loc_foundit != a_filter.end());
                if (!loc_found)
                {
                    (*a_controls)[i].push_back(it);
                } 
            }
        }
    }
}

void UD::ControlManager::ApplyControls(RE::BSTArray<RE::ControlMap::UserEventMapping>* a_controls)
{
    if (a_controls == nullptr) return;
    auto loc_control = RE::ControlMap::GetSingleton()->controlMap[RE::UserEvents::INPUT_CONTEXT_IDS::kGameplay]->deviceMappings;
    for (int i = 0; i <= RE::INPUT_DEVICES::kVirtualKeyboard; i++)
    {
        loc_control[i] = a_controls[i];
    }
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

        const bool loc_bound = ControlManager::GetSingleton()->PlayerIsBound();
        if ( loc_bound                                                  &&
            !ControlManager::GetSingleton()->PlayerInMinigame()         &&
            !ControlManager::GetSingleton()->PlayerInZadAnimation())
        {
            const RE::INPUT_DEVICE loc_Device = loc_buttonEvent->GetDevice();
            const bool loc_hmbuttonpress = ControlManager::GetSingleton()->HardcoreButtonPressed(loc_dxScanCode,loc_Device);
            if (loc_hmbuttonpress)
            {
                if (loc_bound)
                {
                    RE::DebugNotification("You are bound and can't do anything!");
                }
                ModEvents::GetSingleton()->HMTweenMenuEvent.QueueEvent();

                UDSKSELOG("Sending player tween menu event")

                std::thread([this]()
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

RE::BSEventNotifyControl UD::CameraEventSink::ProcessEvent(const SKSE::CameraEvent* eventPtr, RE::BSTEventSource<SKSE::CameraEvent>*)
{                      
    if (eventPtr == nullptr) return RE::BSEventNotifyControl::kContinue;
    
    

    RE::TESCameraState* loc_new = eventPtr->newState;
    RE::TESCameraState* loc_old = eventPtr->oldState;
    

    if (loc_new != nullptr)
    {
        UDSKSELOG("Camera state: new = {}",loc_new->id);
        switch(loc_new->id)
        {
        case RE::CameraState::kFree:
            ControlManager::GetSingleton()->DisableControlsFC();
            return RE::BSEventNotifyControl::kContinue;
            break;
        }
    }

    if (loc_old != nullptr)
    {
        UDSKSELOG("Camera state: old = {}",loc_old->id);
        switch(loc_old->id)
        {
        case RE::CameraState::kFree:
            ControlManager::GetSingleton()->DisableControls(); //disable untill control manager is updated
            return RE::BSEventNotifyControl::kContinue;
            break;
        }
    }

    return RE::BSEventNotifyControl::kContinue;
}
