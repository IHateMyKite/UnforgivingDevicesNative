#pragma once

namespace UD
{
    enum LockpickVariable : int32_t
    {
        vpickKeyTime        =  0,
        vlockKeyTime        =  1,
        vpickAngle          =  2,
        vlockAngle          =  3,
        vdamagePickAngle    =  4,
        vpickBreakSeconds   =  5,
        vsweetSpotAngle     =  6,
        vpartialPickAngle   =  7,
        vnumBrokenPicks     =  8,
        vanimating          =  9,
        vmenuCleared        = 10,
        vanimationFinished  = 11,
        visLockpickingCrime = 12

    };

    class MenuEventSink : public RE::BSTEventSink<RE::MenuOpenCloseEvent>
    {
    SINGLETONHEADER(MenuEventSink)
    public:
        RE::BSEventNotifyControl ProcessEvent(const RE::MenuOpenCloseEvent* a_event, RE::BSTEventSource<RE::MenuOpenCloseEvent>* a_eventSource);
    };

    class LockpickManager
    {
    SINGLETONHEADER(LockpickManager)
    public:
        void Setup();
        float GetLockpickVariable(LockpickVariable a_var) const;
        bool  SetLockpickVariable(LockpickVariable a_var, float a_value) const;
        int  GetDestroyedLockpicks() const;
        bool SetLockpickIsCrime(bool a_val) const;
    private:
        bool _installed = false;
    };

    inline int GetDestroyedLockpicks(PAPYRUSFUNCHANDLE)
    {
        return LockpickManager::GetSingleton()->GetDestroyedLockpicks();
    }

    inline bool SetLockpickIsCrime(PAPYRUSFUNCHANDLE, bool a_val)
    {
        return LockpickManager::GetSingleton()->SetLockpickIsCrime(a_val);
    }

    inline float GetLockpickVariable(PAPYRUSFUNCHANDLE, int a_var)
    {
        return LockpickManager::GetSingleton()->GetLockpickVariable((LockpickVariable)a_var);
    }

    inline bool SetLockpickVariable(PAPYRUSFUNCHANDLE, int a_var, float a_val)
    {
        return LockpickManager::GetSingleton()->SetLockpickVariable((LockpickVariable)a_var,a_val);
    }
}