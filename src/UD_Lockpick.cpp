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
    }
}

float UD::LockpickManager::GetLockpickVariable(LockpickVariable a_var) const
{
    auto loc_lockpickmenu = RE::UI::GetSingleton()->GetMenu("Lockpicking Menu");
    if (loc_lockpickmenu)
    {
        RE::LockpickingMenu* loc_lockpickmenuPtr = reinterpret_cast<RE::LockpickingMenu*>(loc_lockpickmenu.get());
        auto loc_rnd = loc_lockpickmenuPtr->GetRuntimeData();

        #define GetLockpickVariable_CASE(var)   \
        case LockpickVariable::v##var:          \
            return loc_rnd.var;                 \
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
        auto loc_rnd = loc_lockpickmenuPtr->GetRuntimeData();

        #define SetLockpickVariable_CASE(var)   \
        case LockpickVariable::v##var:          \
            loc_rnd.var = a_value;              \
            break;

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

int UD::LockpickManager::GetDestroyedLockpicks() const
{
    auto loc_lockpickmenu = RE::UI::GetSingleton()->GetMenu("Lockpicking Menu");
    if (loc_lockpickmenu)
    {
        RE::LockpickingMenu* loc_lockpickmenuPtr = reinterpret_cast<RE::LockpickingMenu*>(loc_lockpickmenu.get());
        return loc_lockpickmenuPtr->GetRuntimeData().numBrokenPicks;
    }
    return 0;
}

bool UD::LockpickManager::SetLockpickIsCrime(bool a_val) const
{
    auto loc_lockpickmenu = RE::UI::GetSingleton()->GetMenu("Lockpicking Menu");
    if (loc_lockpickmenu)
    {
        RE::LockpickingMenu* loc_lockpickmenuPtr = reinterpret_cast<RE::LockpickingMenu*>(loc_lockpickmenu.get());
        loc_lockpickmenuPtr->GetRuntimeData().isLockpickingCrime = a_val;
        return true;
    }
    return false;
}

RE::BSEventNotifyControl UD::MenuEventSink::ProcessEvent(const RE::MenuOpenCloseEvent* a_event, RE::BSTEventSource<RE::MenuOpenCloseEvent>* a_eventSource)
{
    if (a_event)
    {
        LOG("UD::MenuEventSink::ProcessEvent - Menu={}, Opening={}",a_event->menuName,a_event->opening)
        if (a_event->menuName == "Lockpicking Menu")
        {
            auto loc_player     = RE::PlayerCharacter::GetSingleton();
            auto loc_utility    = Utility::GetSingleton();
            auto loc_status     = PlayerStatus::GetSingleton()->GetPlayerStatus();
            // prevent lockpick minigame from working if player have either bondage mittens 
            // or heavy bondage which hides hands (everything except for yoke)
            // also is only relevant if player is not in lockpick minigame (for obvious reasons)
            if (!(loc_status & PlayerStatus::Status::sMinigame) && 
                loc_player && a_event->opening && (loc_utility->WornHasKeyword(loc_player,"zad_DeviousBondageMittens") ||
                (loc_utility->WornHasKeyword(loc_player,"zad_DeviousHeavyBondage") && !loc_utility->WornHasKeyword(loc_player,"zad_DeviousYoke"))
                )
            )
            {
                auto loc_lockpickmenu = RE::UI::GetSingleton()->GetMenu(a_event->menuName);
                if (loc_lockpickmenu)
                {
                    RE::LockpickingMenu* loc_lockpickmenuPtr = reinterpret_cast<RE::LockpickingMenu*>(loc_lockpickmenu.get());
                    if (loc_status & PlayerStatus::Status::sHaveTelekinesis)
                    {
                        RE::DebugNotification("You use telekinesis help with lockpicking");
                        loc_lockpickmenuPtr->GetRuntimeData().sweetSpotAngle    *= 0.25f;
                        loc_lockpickmenuPtr->GetRuntimeData().partialPickAngle  *= 0.5f;
                    }
                    else
                    {
                        RE::DebugNotification("You can't lockpick the lock in your current state!");
                        loc_lockpickmenuPtr->GetRuntimeData().sweetSpotAngle = 0.0f;
                        loc_lockpickmenuPtr->GetRuntimeData().partialPickAngle = 0.0f;
                    }
                    return RE::BSEventNotifyControl::kStop;
                }
            }
        }
    }
    return RE::BSEventNotifyControl::kContinue;
}