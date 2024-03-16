#include <UD_ControlManager.h>


SINGLETONBODY(UD::KeyEventSink)
SINGLETONBODY(UD::CameraEventSink)
SINGLETONBODY(UD::ControlManager)

using RE::BSScript::Variable;

void UD::ControlManager::Setup()
{
    if (!_installed)
    {
        _installed = true;

        //load memory
        _OriginalControls = new RE::BSTArray<RE::ControlMap::UserEventMapping>[4];
        SaveOriginalControls();

        _hardcoreids = Config::GetSingleton()->GetArray<std::string>("Disabler.asHardcoreModeDisable");
        _hardcodemessages = Config::GetSingleton()->GetArrayText("Disabler.asHardcoreMessages",false);

        LOG("Hardcore disable config loaded. Number = {}",_hardcoreids.size())
        for (auto&& it : _hardcoreids) LOG("{}",it)

        InitControlOverride(&_HardcoreControls,_hardcoreids);

        _disableids = _disablenomoveids; //copy disable which doesnt disable movement
        _disableids.insert(_disableids.begin(),{"Forward","Back","Strafe Right","Strafe Left"}); //add movement disable
        for (auto& it : _disableids) std::transform(it.begin(), it.end(), it.begin(), ::tolower);
        for (auto& it : _disablenomoveids) std::transform(it.begin(), it.end(), it.begin(), ::tolower);

        InitControlOverride(&_DisabledControls,_disableids);
        InitControlOverride(&_DisabledNoMoveControls,_disablenomoveids);

        SKSE::GetCameraEventSource()->AddEventSink(CameraEventSink::GetSingleton());

        LOG("ControlManager installed")
        //DebugPrintControls(_OriginalControls);
        //DebugPrintControls(_HardcoreControls);
        //DebugPrintControls(_DisabledControls);
        //DebugPrintControls(_DisabledNoMoveControls);
    }

    _DisableFreeCamera = Config::GetSingleton()->GetVariable<bool>("Disabler.bDisableFreeCamera",true);
}

void UD::ControlManager::UpdateControl()
{
    //check if player is in minigame
    typedef UD::PlayerStatus::Status Status;
    const Status loc_status = PlayerStatus::GetSingleton()->GetPlayerStatus();

    const bool loc_bound        = loc_status & Status::sBound;
    const bool loc_minigame     = loc_status & Status::sMinigame;
    const bool loc_animation    = loc_status & Status::sAnimation;

    //CheckStatusSafe(loc_status);

    if ((loc_minigame || loc_animation))
    {
        if (!_ControlsDisabled)
        {
            DisableControls();
            _ControlsDisabled = true;
        }
    }
    else
    {
        if (_ControlsDisabled)
        {
             ApplyOriginalControls();
            _ControlsDisabled       = false;
            _HardcoreModeApplied    = false;
        }
        if (!_ControlsDisabled)
        {
            if (_hardcoreMode && loc_bound && !_HardcoreModeApplied)
            {
                ApplyControls(_HardcoreControls);
                _HardcoreModeApplied = true;
            }
            else if ((!_hardcoreMode || !loc_bound) && _HardcoreModeApplied)
            {
                ApplyOriginalControls();
                _HardcoreModeApplied = false;
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

void UD::ControlManager::DebugPrintControls(RE::BSTArray<RE::ControlMap::UserEventMapping>* a_controls)
{
    if (a_controls == nullptr) return;

    DEBUG("==Printing controls , Size={}==",a_controls->size())
    for (int j = 0; j <= CONTROLSDISABLE; j++)
    {
        DEBUG("=INPUT_DEVICES = {:2}",j)
        for (auto&& it : a_controls[j]) 
        {
            DEBUG("{:20} , {:6} , {:5} , {:5} , {:6} , {:08X} , {:3}",it.eventID,it.inputKey,it.linked,it.remappable,it.modifier,it.userEventGroupFlag.underlying(),it.indexInContext)
        }
    }
}

void UD::ControlManager::DebugPrintControls()
{
    DEBUG("==Printing control==")
    for (int i = 0; i < RE::UserEvents::INPUT_CONTEXT_IDS::kTotal; i++)
    {
        DEBUG("=INPUT_CONTEXT_IDS = {:2}",i)
        auto loc_control = RE::ControlMap::GetSingleton()->controlMap[i]->deviceMappings;
        for (int j = 0; j <= CONTROLSDISABLE; j++)
        {
            DEBUG("=INPUT_DEVICES = {:2}",j)
            for (auto&& it : loc_control[j]) 
            {
                DEBUG("{:20} , {:6} , {:5} , {:5} , {:6} , {:08X} , {:3}",it.eventID,it.inputKey,it.linked,it.remappable,it.modifier,it.userEventGroupFlag.underlying(),it.indexInContext)
            }
        }
    }
}

bool UD::ControlManager::HardcoreButtonPressed(uint32_t a_dxkeycode, RE::INPUT_DEVICE a_device)
{
    for (int i = 0; i <= CONTROLSDISABLE; i++)
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
    LOG("ApplyOriginalControls called")
    ApplyControls(_OriginalControls);
}

void UD::ControlManager::DisableControls()
{
    LOG("DisableControls called")
    ApplyControls(_DisabledControls);
}

void UD::ControlManager::DisableControlsFC()
{
    LOG("DisableControlsFC called")
    if (_DisableFreeCamera)
    {
        ApplyControls(_DisabledNoMoveControls);
        _ControlsDisabled = true;
    }
    else
    {
        ApplyOriginalControls();
        _ControlsDisabled       = false;
        _HardcoreModeApplied    = false;
    }
}

const std::vector<std::string>& UD::ControlManager::GetHardcoreMessages() const
{
    return _hardcodemessages;
}

bool UD::ControlManager::RegisterDeviceCallback(int a_handle1, int a_handle2, RE::TESObjectARMO* a_device, int a_dxkeycode, std::string a_callbackfun)
{
    LOG("RegisterDeviceCallback({},{},{},{},{}) called",a_handle1,a_handle2,a_device ? a_device->GetFormID() : 0,a_dxkeycode,a_callbackfun)

    if (a_device == nullptr || (a_handle1 == 0 && a_handle2 == 0) || a_callbackfun == "" || a_dxkeycode == 0) return false;
    auto loc_device = PapyrusDelegate::GetSingleton()->GetDeviceScript(a_handle1,a_handle2,a_device);

    if (loc_device.object == nullptr || loc_device.id == nullptr || loc_device.rd == nullptr) return false;
    _DeviceCallbacks[a_dxkeycode] = {loc_device,a_callbackfun};

    LOG("RegisterDeviceCallback({},{},{},{},{}) - Callback registered!",a_handle1,a_handle2,a_device ? a_device->GetFormID() : 0,a_dxkeycode,a_callbackfun)

    return true;
}

bool UD::ControlManager::AddDeviceCallbackArgument(int a_dxkeycode, CallbackArgFuns a_type, std::string a_argStr, RE::TESForm* a_argForm)
{
    LOG("AddDeviceCallbackArgument({},{},{},0x{:08X}) called",a_dxkeycode,(int)a_type,a_argStr,a_argForm ? a_argForm->GetFormID() : 0)

    if (a_dxkeycode == 0) return false;

    auto loc_it = _DeviceCallbacks.find(a_dxkeycode);
    
    if (loc_it == _DeviceCallbacks.end()) return false;

    DeviceCallback* loc_callback = &loc_it->second;

    AddArgument(loc_callback,a_type,a_argStr,a_argForm);

    return true;
}

bool UD::ControlManager::UnregisterDeviceCallbacks(int a_handle1, int a_handle2, RE::TESObjectARMO* a_device)
{
    LOG("UnregisterDeviceCallbacks({},{},0x{:08X}) called",a_handle1,a_handle2,a_device ? a_device->GetFormID() : 0)

    if (a_device == nullptr || (a_handle1 == 0 && a_handle2 == 0)) return false;
    auto loc_device = PapyrusDelegate::GetSingleton()->GetDeviceScript(a_handle1,a_handle2,a_device);

    if (loc_device.object == nullptr || loc_device.id == nullptr || loc_device.rd == nullptr) return false;

    std::erase_if(_DeviceCallbacks,[&](std::pair<const uint32_t,DeviceCallback> a_val)
    {
        auto const& [dxcode, devicecallback] = a_val;
        return devicecallback.device.object.get() == loc_device.object.get();
    });

    LOG("UnregisterDeviceCallbacks({},{},0x{:08X}) - Callbacks removed",a_handle1,a_handle2,a_device ? a_device->GetFormID() : 0)

    return true;
}

void UD::ControlManager::UnregisterAllDeviceCallbacks()
{
    _DeviceCallbacks.clear();
}

const std::unordered_map<uint32_t, UD::DeviceCallback>& UD::ControlManager::GetDeviceCallbacks()
{
    return _DeviceCallbacks;
}

void UD::ControlManager::AddArgument(DeviceCallback* a_callback, CallbackArgFuns a_funtype, std::string a_argStr, RE::TESForm* a_argForm)
{
    if (a_callback == nullptr) return;
    CallbackArgType loc_atype;
    std::function<void*(std::string a_argStr, RE::TESForm* a_argForm)> loc_fun;
    switch(a_funtype)
    {
    case fWidgetValue:
        loc_atype = tFloat;
        loc_fun = [](std::string a_argStr, RE::TESForm* a_argForm) -> void*
        {
            auto loc_param = GetStringParamAllInter<std::string>(a_argStr,",");
            if (loc_param.size() != 2) return nullptr;
            float* loc_res = nullptr;
            if (loc_param[0] == "0") 
            {
                loc_res = new float(MeterManager::GetMeterValueSkyUi(loc_param[1]));
                MeterManager::SetMeterValueSkyUi(loc_param[1],0.0f);
            }
            else if (loc_param[0] == "1") 
            {
                loc_res = new float(MeterManager::GetMeterValueIWW(std::stoi(loc_param[1])));
                MeterManager::SetMeterValueIWW(std::stoi(loc_param[1]),0.0f);
            }
            return loc_res;
        };
        break;
    default:
        return;
        break;
    }

    a_callback->args.push_back({a_funtype,loc_atype,a_argStr,a_argForm,loc_fun});
}

void UD::ControlManager::SaveOriginalControls()
{
    LOG("SaveOriginalControls called")
    auto loc_control = RE::ControlMap::GetSingleton()->controlMap[RE::UserEvents::INPUT_CONTEXT_IDS::kGameplay]->deviceMappings;
    for (int i = 0; i <= CONTROLSDISABLE; i++)
    {
        _OriginalControls[i] = loc_control[i];
    }
}

void UD::ControlManager::InitControlOverride(RE::BSTArray<RE::ControlMap::UserEventMapping>** a_controls,const std::vector<std::string>& a_filter)
{
    *a_controls = new RE::BSTArray<RE::ControlMap::UserEventMapping>[CONTROLSDISABLE + 1];

    auto loc_control = RE::ControlMap::GetSingleton()->controlMap[RE::ControlMap::InputContextID::kGameplay]->deviceMappings;
    for (int i = 0; i <= CONTROLSDISABLE; i++)
    {
        for (auto&& it : loc_control[i]) 
        {
            {
                const auto loc_foundit = std::find_if(a_filter.begin(),a_filter.end(),[it](const std::string& a_id)
                {
                    std::string loc_eventstr = it.eventID.c_str();
                    std::transform(loc_eventstr.begin(), loc_eventstr.end(), loc_eventstr.begin(), ::tolower);
                    if (loc_eventstr == a_id) return true;
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
    for (int i = 0; i <= CONTROLSDISABLE; i++)
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
        LOG("KeyEventSink::ProcessEvent(...) dxkeycode = {}",loc_dxScanCode)

        auto loc_callbacks = ControlManager::GetSingleton()->GetDeviceCallbacks();
        const auto loc_callback = loc_callbacks.find(loc_dxScanCode);
        if (loc_callback != loc_callbacks.end())
        {
            LOG("KeyEventSink::ProcessEvent(...) Device callback found - Calling function",loc_dxScanCode)
            loc_callback->second.Send();
            return RE::BSEventNotifyControl::kContinue;
        }

        typedef UD::PlayerStatus::Status Status;
        const Status loc_status = PlayerStatus::GetSingleton()->GetPlayerStatus();

        const bool loc_bound = loc_status & Status::sBound;
        if ( loc_bound && !(loc_status & Status::sMinigame) && !(loc_status & Status::sAnimation))
        {
            const RE::INPUT_DEVICE loc_Device = loc_buttonEvent->GetDevice();
            const bool loc_hmbuttonpress = ControlManager::GetSingleton()->HardcoreButtonPressed(loc_dxScanCode,loc_Device);
            if (loc_hmbuttonpress)
            {
                if (loc_bound)
                {
                    auto loc_messages = ControlManager::GetSingleton()->GetHardcoreMessages();
                    if (loc_messages.size() > 0 && loc_messages[0] != "")
                    {
                        RE::DebugNotification(loc_messages[RandomGenerator::GetSingleton()->RandomInt(0,(int)loc_messages.size() - 1)].c_str());
                    }
                }
                ModEvents::GetSingleton()->HMTweenMenuEvent.QueueEvent();

                LOG("Sending player tween menu event")

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
    
    _state = loc_new;

    if (loc_new != nullptr)
    {
        LOG("Camera state: new = {}",loc_new->id);
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
        LOG("Camera state: old = {}",loc_old->id);
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

int UD::CameraEventSink::GetCameraState() const
{
    if (_state) return _state->id;
    return -1;
}

void UD::DeviceCallback::Send()
{
    ////init unused callback
    RE::BSTSmartPointer<RE::BSScript::IStackCallbackFunctor> loc_funcallback;
    static const auto loc_vm = InternalVM::GetSingleton();

    auto loc_args = new VarFunctionArguments();
    loc_args->SetArgs(args);

    ////call papyrus method
    loc_vm->DispatchMethodCall(device.object,callback,loc_args,loc_funcallback);
    delete loc_args;
}

bool UD::VarFunctionArguments::operator()(RE::BSScrapArray<Variable>& a_dst) const
{
    a_dst.resize(_args.size());
    for (int i = 0; i < _args.size(); i++)
    {
        auto loc_callback = _args[i];

        void* loc_res = loc_callback.fun(loc_callback.argStr,loc_callback.argForm);
        if (loc_res == nullptr) continue;

        RE::BSScript::Variable loc_var;
        switch(loc_callback.type)
        {
        case tInt:
            a_dst[i].SetSInt(*(int*)loc_res);
            break;
        case tFloat:
            a_dst[i].SetFloat(*(float*)loc_res);
            break;
        case tString:
            a_dst[i].SetString(*(std::string*)loc_res);
            break;
        case tForm:
            break;
        }
        delete loc_res;
    }
    return true;
}
