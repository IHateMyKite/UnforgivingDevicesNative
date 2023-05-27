//multiplier used for converting existing papyrus over time effects to native over time effect
//as native effects are much faster, we need to reduce its amplitude, otherwise it will be much stronger then papyrus one
#define UDCONVERTMULT 0.5f

//handle of papyrus functions
#define PAPYRUSFUNCHANDLE RE::BSScript::Internal::VirtualMachine* a_vm, const RE::VMStackID a_stackID, RE::StaticFunctionTag*

#define UDTRUNCVALUE(val,min,max)       \
{                                       \
    if (val > max) val = max;           \
    else if (val < min) val = min;      \
}