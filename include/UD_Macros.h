//debug build
#define UDDEBUG 1U

//multiplier used for converting existing papyrus over time effects to native over time effect
//as native effects are much faster, we need to reduce its amplitude, otherwise it will be much stronger then papyrus one
#define UDCONVERTMULT 0.4f

//if skyui meters should force the new value (play the animation)
#define UDSKYUIFORCE 1.0f

//handle of papyrus functions
#define PAPYRUSFUNCHANDLE RE::BSScript::Internal::VirtualMachine* a_vm, const RE::VMStackID a_stackID, RE::StaticFunctionTag*

#define UDTRUNCVALUE(val,min,max)       \
{                                       \
    if (val > max) val = max;           \
    else if (val < min) val = min;      \
}

#if(UDDEBUG == 1U)
    #define UDCONSOLELOG(...) {RE::ConsoleLog::GetSingleton()->Print(std::format(__VA_ARGS__).c_str());}
#else
    #define UDCONSOLELOG(...) {}
#endif

#define UDSKSELOG(...) {SKSE::log::info(__VA_ARGS__);}