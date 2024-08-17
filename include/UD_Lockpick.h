#pragma once
#include <xbyak/xbyak.h>

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

    struct PatchLockpickCrimeDisable : public Xbyak::CodeGenerator
    {
        PatchLockpickCrimeDisable()
        {
            nop(5,false); //noped call for some function which adds bounty and makes guards approach player
            mov(byte [rbx + 0x10D],0U); //originaly was setting variable to 1, which was forcing the menu to close if player was detected
        }
    };

    class LockpickManager
    {
    SINGLETONHEADER(LockpickManager)
    public:
        void Setup();
        float GetLockpickVariable(LockpickVariable a_var) const;
        bool  SetLockpickVariable(LockpickVariable a_var, float a_value) const;
        void DisableLockpickCrime();
        void EnableLockpickCrime();
    private:
        bool _installed = false;
        bool _PatchLockpickCrimeApplied = false;
        uint8_t _PatchLockpickCrimeOld[100]; //buffer for storing original data so we can restore it later
        REL::Relocation<std::uintptr_t> _PatchLockpickCrimeAddr;
        PatchLockpickCrimeDisable       _PatchLockpickCrimeDisable;
    };

    inline float GetLockpickVariable(PAPYRUSFUNCHANDLE, int a_var)
    {
        return LockpickManager::GetSingleton()->GetLockpickVariable((LockpickVariable)a_var);
    }

    inline bool SetLockpickVariable(PAPYRUSFUNCHANDLE, int a_var, float a_val)
    {
        return LockpickManager::GetSingleton()->SetLockpickVariable((LockpickVariable)a_var,a_val);
    }
}