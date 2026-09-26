#include "UD_Lockpick.h"

#include <UD_Utility.h>
#include <UD_Config.h>
#include <UD_PlayerStatus.h>

SINGLETONBODY(UD::LockpickManager)
SINGLETONBODY(UD::MenuEventSink)

void UD::LockpickManager::Setup()
{
    if (!_installed)
    {
        _installed = true;
        if (RE::UI::GetSingleton())
        {
            RE::UI::GetSingleton()->AddEventSink<RE::MenuOpenCloseEvent>(MenuEventSink::GetSingleton());
            DEBUG("Menu sink installed")
        }

        // SE: 14089918E
        // AE: 140939B52
        // VR: 1408c6f8e
        if (REL::Module::IsVR()) {
            _PatchLockpickCrimeAddr = (std::uintptr_t)REL::Offset(0x8c6f8e).address();
        } else {
            _PatchLockpickCrimeAddr = REL::Relocation<std::uintptr_t>{REL::RelocationID(51099, 51981), REL::VariantOffset(0xEEU, 0xF2U, 0xF2U)};
        }
        _PatchLockpickCrimeDisable.ready();
        _PatchLockpickCrimeDisableAE1p7.ready();
        DEBUG("_PatchLockpickCrimeAddr = 0x{:016X}",_PatchLockpickCrimeAddr.address())

        
        
        if (REL::Module::get().version() > REL::Version("1.7.0.0")) {
            DEBUG("_PatchLockpickCrimeDisableAE1p7 size = 0x{:02X}",_PatchLockpickCrimeDisableAE1p7.getSize())
            //store original code
            std::memcpy(_PatchLockpickCrimeOld,(uintptr_t*)_PatchLockpickCrimeAddr.address(),_PatchLockpickCrimeDisableAE1p7.getSize());

            if (_PatchLockpickCrimeDisableAE1p7.getSize() > 0xCU)
            {
                SKSE::stl::report_and_fail("UnforgivingDevices: PatchLockpickCrimeDisableAE1p7 was too large, failed to install"sv);
            }
                    std::string loc_rawdata = "";
            for (size_t i = 0; i < _PatchLockpickCrimeDisableAE1p7.getSize(); i++)
            {
                loc_rawdata += std::format(" {:02X}",_PatchLockpickCrimeOld[i]);
            }
            DEBUG("_PatchLockpickCrimeOld = {}",loc_rawdata)
        } else {
            DEBUG("PatchLockpickCrimeDisable size = 0x{:02X}",_PatchLockpickCrimeDisable.getSize())
            //store original code
            std::memcpy(_PatchLockpickCrimeOld,(uintptr_t*)_PatchLockpickCrimeAddr.address(),_PatchLockpickCrimeDisable.getSize());

            if (_PatchLockpickCrimeDisable.getSize() > 0xCU)
            {
                SKSE::stl::report_and_fail("UnforgivingDevices: PatchLockpickCrimeDisable was too large, failed to install"sv);
            }
                    std::string loc_rawdata = "";
            for (size_t i = 0; i < _PatchLockpickCrimeDisable.getSize(); i++)
            {
                loc_rawdata += std::format(" {:02X}",_PatchLockpickCrimeOld[i]);
            }
            DEBUG("_PatchLockpickCrimeOld = {}",loc_rawdata)
        }
        


    }
}

float UD::LockpickManager::GetLockpickVariable(LockpickVariable a_var) const
{
    auto loc_lockpickmenu = RE::UI::GetSingleton()->GetMenu("Lockpicking Menu");
    if (loc_lockpickmenu)
    {
        RE::LockpickingMenu* loc_lockpickmenuPtr = reinterpret_cast<RE::LockpickingMenu*>(loc_lockpickmenu.get());
        auto& loc_rnd = loc_lockpickmenuPtr->GetRuntimeData();
        uint64_t pick_offset=0x0;
        if (REL::Module::get().version() > REL::Version("1.7.0.0")) {
            if ((uintptr_t)(&loc_rnd.numBrokenPicks)-(uintptr_t)(&loc_rnd) == 0xBC) {
                pick_offset=(uint64_t)0x14;
            }
            
        }
        #define GetLockpickVariable_CASE(var)                 \
        case LockpickVariable::v##var:                        \
            {                                                   \
                decltype(loc_rnd.var)* p = static_cast<decltype(loc_rnd.var)*>(&(loc_rnd.var));                         \
                if (((uint64_t)p-(uint64_t)&loc_rnd) >= 0xb8) { \
                    p += pick_offset/sizeof(decltype(loc_rnd.var));                          \
                }    \
                return *p;                                        \
            }                                                   \
            break;

        switch (a_var)
        {
            GetLockpickVariable_CASE(pickKeyTime        )
            GetLockpickVariable_CASE(pickAngle          )
            GetLockpickVariable_CASE(lockKeyTime        )
            GetLockpickVariable_CASE(lockAngle          )
            GetLockpickVariable_CASE(damagePickAngle    )
            GetLockpickVariable_CASE(pickBreakSeconds   )
            GetLockpickVariable_CASE(sweetSpotAngle     )
            GetLockpickVariable_CASE(partialPickAngle   )
            GetLockpickVariable_CASE(numBrokenPicks     )
            GetLockpickVariable_CASE(animating          )
            GetLockpickVariable_CASE(menuCleared        )
            GetLockpickVariable_CASE(animationFinished  )
            GetLockpickVariable_CASE(isLockpickingCrime )
            default:
                return 0.0f;
        }

        #undef GetLockpickVariable_CASE
    }

    return 0.0f;
}

bool UD::LockpickManager::SetLockpickVariable(LockpickVariable a_var, float a_value) const
{
    auto loc_lockpickmenu = RE::UI::GetSingleton()->GetMenu("Lockpicking Menu");
    if (loc_lockpickmenu)
    {
        RE::LockpickingMenu* loc_lockpickmenuPtr = reinterpret_cast<RE::LockpickingMenu*>(loc_lockpickmenu.get());
        auto& loc_rnd = loc_lockpickmenuPtr->GetRuntimeData();
        uint64_t pick_offset=(uint64_t)0x0;
        if (REL::Module::get().version() > REL::Version("1.7.0.0")) {
            if ((uintptr_t)(&loc_rnd.numBrokenPicks) - (uintptr_t)(&loc_rnd) == 0xBC) {
                pick_offset=(uint64_t)0x14;
            }
            
        }
        #define SetLockpickVariable_CASE(var)   \
        case LockpickVariable::v##var:          \
            {                                   \
                decltype(loc_rnd.var)* p = static_cast<decltype(loc_rnd.var)*>(&(loc_rnd.var));  \
                uint64_t* raw_p=(uint64_t*)&p;                            \
                if (((uint64_t)p-(uint64_t)&loc_rnd) >= 0xb8) { \
                    *raw_p += pick_offset; \
                }                                                   \
                *p = a_value;                                       \
            }                                                   \
            return true;
    
        switch (a_var)
        {
            SetLockpickVariable_CASE(pickKeyTime        )
            SetLockpickVariable_CASE(pickAngle          )
            SetLockpickVariable_CASE(lockKeyTime        )
            SetLockpickVariable_CASE(lockAngle          )
            SetLockpickVariable_CASE(damagePickAngle    )
            SetLockpickVariable_CASE(pickBreakSeconds   )
            SetLockpickVariable_CASE(sweetSpotAngle     )
            SetLockpickVariable_CASE(partialPickAngle   )
            SetLockpickVariable_CASE(numBrokenPicks     )
            SetLockpickVariable_CASE(animating          )
            SetLockpickVariable_CASE(menuCleared        )
            SetLockpickVariable_CASE(animationFinished  )
            SetLockpickVariable_CASE(isLockpickingCrime )
            default:
                return false;
        }
    
        #undef GetLockpickVariable_CASE
    }

    return false;
}

void UD::LockpickManager::DisableLockpickCrime()
{
    if (!_PatchLockpickCrimeApplied)
    {
        _PatchLockpickCrimeApplied = true;

        if (REL::Module::get().version() > REL::Version("1.7.0.0")) {
            REL::safe_fill(_PatchLockpickCrimeAddr.address(), REL::NOP, _PatchLockpickCrimeDisableAE1p7.getSize());
            REL::safe_write(_PatchLockpickCrimeAddr.address(), _PatchLockpickCrimeDisableAE1p7.getCode(), _PatchLockpickCrimeDisableAE1p7.getSize());
        } else {
            REL::safe_fill(_PatchLockpickCrimeAddr.address(), REL::NOP, _PatchLockpickCrimeDisable.getSize());
            REL::safe_write(_PatchLockpickCrimeAddr.address(), _PatchLockpickCrimeDisable.getCode(), _PatchLockpickCrimeDisable.getSize());
        }
    }
}

void UD::LockpickManager::EnableLockpickCrime()
{
    if (_PatchLockpickCrimeApplied)
    {
        if (REL::Module::get().version() > REL::Version("1.7.0.0")) {
            REL::safe_fill(_PatchLockpickCrimeAddr.address(), REL::NOP, _PatchLockpickCrimeDisableAE1p7.getSize());
            REL::safe_write(_PatchLockpickCrimeAddr.address(), _PatchLockpickCrimeOld, _PatchLockpickCrimeDisableAE1p7.getSize());
        } else {
            REL::safe_fill(_PatchLockpickCrimeAddr.address(), REL::NOP, _PatchLockpickCrimeDisable.getSize());
            REL::safe_write(_PatchLockpickCrimeAddr.address(), _PatchLockpickCrimeOld, _PatchLockpickCrimeDisable.getSize());
        }
        _PatchLockpickCrimeApplied = false;
    }
}

RE::BSEventNotifyControl UD::MenuEventSink::ProcessEvent(const RE::MenuOpenCloseEvent* a_event, RE::BSTEventSource<RE::MenuOpenCloseEvent>* a_eventSource)
{
    if (a_event)
    {
        LOG("MenuEventSink::ProcessEvent - Menu={}, Opening={}",a_event->menuName,a_event->opening)
        if (a_event->menuName == RE::LockpickingMenu::MENU_NAME)
        {
            auto loc_player     = RE::PlayerCharacter::GetSingleton();
            auto loc_utility    = Utility::GetSingleton();
            auto loc_minigame   = PlayerStatus::GetSingleton()->PlayerInMinigame();

            if (loc_minigame)
            {
                if (a_event->opening)
                {
                    LockpickManager::GetSingleton()->DisableLockpickCrime();
                }
                else
                {
                    LockpickManager::GetSingleton()->EnableLockpickCrime();
                }
            }

            // prevent lockpick minigame from working if player have either bondage mittens 
            // or heavy bondage which hides hands (everything except for yoke)
            // also is only relevant if player is not in lockpick minigame (for obvious reasons)
            if (!(loc_minigame) && 
                loc_player && a_event->opening && (loc_utility->WornHasKeyword(loc_player,"zad_DeviousBondageMittens") ||
                (loc_utility->WornHasKeyword(loc_player,"zad_DeviousHeavyBondage") && !loc_utility->WornHasKeyword(loc_player,"zad_DeviousYoke"))
                )
            )
            {
                auto loc_lockpickmenu = RE::UI::GetSingleton()->GetMenu(a_event->menuName);
                if (loc_lockpickmenu)
                {
                    RE::LockpickingMenu* loc_lockpickmenuPtr = reinterpret_cast<RE::LockpickingMenu*>(loc_lockpickmenu.get());
                    auto& loc_rnd = loc_lockpickmenuPtr->GetRuntimeData();
                    uint64_t pick_offset=0x0;
                    if (REL::Module::get().version() > REL::Version("1.7.0.0")) {
                        if ((uintptr_t)(&loc_rnd.numBrokenPicks)-(uintptr_t)(&loc_rnd) == 0xBC) {
                            pick_offset=(uint64_t)0x14;
                        }
                        
                    }
                    if (PlayerStatus::GetSingleton()->PlayerHaveTelekinesis())
                    {
                        RE::SendHUDMessage::ShowHUDMessage("You use telekinesis to help with lockpicking");
                        loc_rnd.sweetSpotAngle    *= 0.25f;
                        (*((float*)(&loc_rnd.partialPickAngle)+(pick_offset/sizeof(float))))  *= 0.5f;
                    }
                    else
                    {
                        RE::SendHUDMessage::ShowHUDMessage("You can't lockpick the lock in your current state!");
                        loc_rnd.sweetSpotAngle = 0.0f;
                        (*((float*)(&loc_rnd.partialPickAngle)+(pick_offset/sizeof(float)))) = 0.0f;
                    }
                    return RE::BSEventNotifyControl::kStop;
                }
            }
        }
    }
    return RE::BSEventNotifyControl::kContinue;
}
