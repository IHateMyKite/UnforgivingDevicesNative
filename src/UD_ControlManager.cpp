#include <UD_ControlManager.h>
#include <UD_Utility.h>
#include <UD_Config.h>
#include <UD_PlayerStatus.h>
#include <UD_UI.h>
#include <UD_ModEvents.h>
#include <UD_PapyrusDelegate.h>

#include <shared_mutex>

SINGLETONBODY(UD::KeyEventSink)
SINGLETONBODY(UD::CameraEventSink)
SINGLETONBODY(UD::ControlManager)

using RE::BSScript::Variable;

void UD::ControlManager::Setup()
{
    if (!_installed)
    {
        _installed = true;

        uint8_t loc_ueflag = Config::GetSingleton()->GetVariable("Disabler.iEventGroupFlag", 13);
        if (loc_ueflag > 12 && loc_ueflag < 30)
        {
            _UDEventGroupFlag = static_cast<UEFlag>(1 << loc_ueflag);
        }
        else
        {
            LOG("Invalid value for Disabler.iEventGroupFlag")
        }

        auto loc_hardcoreids = Config::GetSingleton()->GetArray<std::string>("Disabler.asHardcoreModeDisable");
        _hardcodemessages = Config::GetSingleton()->GetArrayText("Disabler.asHardcoreMessages",false);
        AddToFilter(_hardcoreFilter,loc_hardcoreids);
        LOG("Hardcore disable config loaded. Number = {}",_hardcoreFilter.size())
        for (auto&& it : _hardcoreFilter) LOG("{}", it)

        AddToFilter(_disableFilter,_disableids);
        AddToFilter(_freeCamFilter,_disableids);
        AddToFilter(_freeCamFilter,_movementids);

        SKSE::GetCameraEventSource()->AddEventSink(CameraEventSink::GetSingleton());

        LOG("ControlManager installed")
    }

    _state = cEnable;
    _activeFilter = nullptr;
    _DisableFreeCamera = Config::GetSingleton()->GetVariable<bool>("Disabler.bDisableFreeCamera",true);
    _hardcoreMode = PapyrusDelegate::GetSingleton()->GetScriptVariableBool("UDCustomDeviceMain","UD_HardcoreMode",true);
}

void UD::ControlManager::UpdateControl()
{
    //check if player is in minigame
    typedef UD::PlayerStatus::Status Status;

    PlayerStatus::GetSingleton()->Update();

    const Status loc_status = PlayerStatus::GetSingleton()->GetPlayerStatus();

    const bool loc_bound        = loc_status & Status::sBound;
    const bool loc_minigame     = loc_status & Status::sMinigame;
    const bool loc_animation    = loc_status & Status::sAnimation;

    _hardcoreMode = PapyrusDelegate::GetSingleton()->GetScriptVariableBool("UDCustomDeviceMain","UD_HardcoreMode",true);

    //CheckStatusSafe(loc_status);

    auto loc_state = _state.load();
    uint8_t loc_newstate;
    do {
        loc_newstate = loc_state & cFreeCam;
        if (loc_minigame || loc_animation) loc_newstate |= cDisable;
        if (_hardcoreMode && loc_bound) loc_newstate |= cHardcore;
    } while(!_state.compare_exchange_weak(loc_state, loc_newstate));

    RefreshFilter();
}

bool UD::ControlManager::HardcoreMode() const
{
    return _hardcoreMode;
}

bool UD::ControlManager::HardcoreButtonPressed(uint32_t a_dxkeycode, RE::INPUT_DEVICE a_device)
{
    if (a_device < 0 || a_device > CONTROLSDISABLE) return false;

    const auto controlMap = RE::ControlMap::GetSingleton();
    const auto loc_context = controlMap->controlMap[RE::UserEvents::INPUT_CONTEXT_ID::kGameplay];
    const auto& loc_mappings = loc_context->deviceMappings[a_device];

    for (const auto& it : loc_mappings)
    {
        if (it.inputKey != a_dxkeycode) continue;

        if (std::ranges::contains(_hardcoreFilter.cbegin(), _hardcoreFilter.cend(), it.eventID))
        {
            return true;
        }
    }

    return false;
}

void UD::ControlManager::EnterFreeCam()
{
    LOG("EnterFreeCam called")
    _state.fetch_or(cFreeCam);
    RefreshFilter();
}

void UD::ControlManager::ExitFreeCam()
{
    LOG("ExitFreeCam called")
    _state.fetch_and(~cFreeCam);
    RefreshFilter();
}

const std::vector<std::string>& UD::ControlManager::GetHardcoreMessages() const
{
    return _hardcodemessages;
}

bool UD::ControlManager::RegisterDeviceCallback(int a_handle1, int a_handle2, RE::TESObjectARMO* a_device, int a_dxkeycode, std::string a_callbackfun)
{
    LOG("RegisterDeviceCallback({},{},{},{},{}) called",a_handle1,a_handle2,a_device ? a_device->GetFormID() : 0,a_dxkeycode,a_callbackfun)

    if (a_device == nullptr || (a_handle1 == 0 && a_handle2 == 0) || a_callbackfun == "" || a_dxkeycode == 0) return false;
    PapyrusDelegate::GetSingleton()->Lock();
    auto loc_device = PapyrusDelegate::GetSingleton()->GetDeviceScript(a_handle1,a_handle2,a_device);
    PapyrusDelegate::GetSingleton()->Unlock();
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

    PapyrusDelegate::GetSingleton()->Lock();
    auto loc_device = PapyrusDelegate::GetSingleton()->GetDeviceScript(a_handle1,a_handle2,a_device);
    PapyrusDelegate::GetSingleton()->Unlock();

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

bool UD::ControlManager::HaveDeviceCallbacks() const
{
    return _DeviceCallbacks.size() > 0;
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

void UD::ControlManager::RefreshFilter()
{
    static std::shared_mutex m;

    const auto controlMap = RE::ControlMap::GetSingleton();
    {
        std::unique_lock lock{m};

        const auto loc_context = controlMap->controlMap[RE::UserEvents::INPUT_CONTEXT_IDS::kGameplay];
        auto& loc_mappings = loc_context->deviceMappings;

        const auto loc_state = _state.load();
        assert((loc_state & cMask) == loc_state);

        std::vector<RE::BSFixedString>* loc_filter = nullptr;
        switch (_stateToFilter[loc_state & cMask])
        {
        case ControlState::cDisable:
            loc_filter = &_disableFilter;
            break;
        case ControlState::cHardcore:
            loc_filter = &_hardcoreFilter;
            break;
        case ControlState::cFreeCam:
            if (_DisableFreeCamera) loc_filter = &_freeCamFilter;
            break;
        }

        if (_activeFilter == loc_filter) return;

        for (int i = 0; i <= CONTROLSDISABLE; i++)
        {
            for (auto& it : loc_mappings[i])
            {
                if (loc_filter && std::ranges::contains(loc_filter->cbegin(), loc_filter->cend(), it.eventID))
                {
                    if (it.userEventGroupFlag.all(UEFlag::kInvalid))
                        it.userEventGroupFlag = UEFlag::kNone;
                    it.userEventGroupFlag.set(_UDEventGroupFlag);
                }
                else if (it.userEventGroupFlag.none(UEFlag::kInvalid))
                {
                    it.userEventGroupFlag.reset(_UDEventGroupFlag);
                }
            }
        }
        _activeFilter = loc_filter;
    }
    ToggleControls(controlMap, _UDEventGroupFlag, false);
}

void UD::ControlManager::ToggleControls(RE::ControlMap* controlMap, RE::ControlMap::UEFlag a_flags, bool a_enable)
{
    using UEFlag = RE::UserEvents::USER_EVENT_FLAG;
    using UEFlagEnum = SKSE::stl::enumeration<UEFlag, std::uint32_t>;

    UEFlagEnum* loc_enabledControls = &REL::RelocateMember<UEFlagEnum>(controlMap, 0x118, 0x138);
    UEFlagEnum* loc_unk11C = &REL::RelocateMember<UEFlagEnum>(controlMap, 0x11C, 0x136);
    if SKYRIM_REL_CONSTEXPR (REL::Module::IsAE())
    {
        if (REL::Module::get().version().compare(REL::Version(1, 6, 1130, 0)) != std::strong_ordering::less)
        {
            loc_enabledControls = &REL::RelocateMember<UEFlagEnum>(controlMap, 0x120);
            loc_unk11C =  &REL::RelocateMember<UEFlagEnum>(controlMap, 0x124);
        }
    }

    auto oldState = *loc_enabledControls;

    if (a_enable)
    {
        loc_enabledControls->set(a_flags);
        if (*loc_unk11C != UEFlag::kInvalid)
        {
            loc_unk11C->set(a_flags);
        }
    }
    else
    {
        loc_enabledControls->reset(a_flags);
        if (*loc_unk11C != UEFlag::kInvalid)
        {
            loc_unk11C->reset(a_flags);
        }
    }

    RE::UserEventEnabled event{ *loc_enabledControls, oldState };
    controlMap->SendEvent(std::addressof(event));
}

template <std::ranges::range Range>
requires std::convertible_to<std::ranges::range_value_t<Range>, std::string_view>
void UD::ControlManager::AddToFilter(std::vector<RE::BSFixedString>& a_filter, const Range& a_ids)
{
    const auto userEvents = RE::UserEvents::GetSingleton();
    const auto span = std::span(&userEvents->forward, &userEvents->itemZoom + 1);
    for (auto &id : a_ids)
    {
        //lookup canonical value
        auto loc_it = std::ranges::find_if(span, [&](const RE::BSFixedString& it)
        {
            if (it.empty()) return false;

            return std::ranges::equal(static_cast<std::string_view>(it), id,
                [](char a, char b) { return ::tolower(a) == ::tolower(b); });
        });
        if (loc_it != span.cend())
        {
            a_filter.push_back(*loc_it);
        }
        else
        {
            WARN("Unrecognized UserEvent {}", id)
        }
    }
}

RE::BSEventNotifyControl UD::KeyEventSink::ProcessEvent(RE::InputEvent* const* eventPtr, RE::BSTEventSource<RE::InputEvent*>*)
{
    if (_Cooldown) return RE::BSEventNotifyControl::kContinue; //on cooldown - return
    if (!eventPtr) return RE::BSEventNotifyControl::kContinue;
    if (!ControlManager::GetSingleton()->HardcoreMode() && (!ControlManager::GetSingleton()->HaveDeviceCallbacks())) return RE::BSEventNotifyControl::kContinue; //no hardcore mode - return
    
    auto* event = *eventPtr;
    if (!event) return RE::BSEventNotifyControl::kContinue;

    if (event->GetEventType() == RE::INPUT_EVENT_TYPE::kButton) 
    {
        const auto*       loc_buttonEvent = event->AsButtonEvent();
        const uint32_t    loc_dxScanCode  = loc_buttonEvent->GetIDCode();
        if (loc_buttonEvent->IsRepeating()) return RE::BSEventNotifyControl::kContinue;
        LOG("KeyEventSink::ProcessEvent(...) dxkeycode = {}",loc_dxScanCode)

        static auto loc_utility = Utility::GetSingleton();
        bool loc_ismenuopen = loc_utility->IsBlockingMenuOpen();

        if (loc_ismenuopen) return RE::BSEventNotifyControl::kContinue;

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
            ControlManager::GetSingleton()->EnterFreeCam();
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
            ControlManager::GetSingleton()->ExitFreeCam();
            return RE::BSEventNotifyControl::kContinue;
            break;
        }
    }

    ControlManager::GetSingleton()->UpdateControl();

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
