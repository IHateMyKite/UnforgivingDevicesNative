#include <OrgasmSystem\OrgasmEvents.h>

SINGLETONBODY(ORS::OrgasmEvents)

void ORS::OrgasmEvents::RegisterPapyrus(RE::BSScript::IVirtualMachine* vm)
{
    #define REGISTERPAPYRUSFUNC(name,unhook) vm->RegisterFunction(#name, "OrgasmSystem", ORS::name,unhook);
    // ----
    REGISTERPAPYRUSFUNC(RegisterForOrgasmEvent_Ref,true)
    REGISTERPAPYRUSFUNC(RegisterForOrgasmEvent_Form,true)
    REGISTERPAPYRUSFUNC(RegisterForExpressionUpdate_Ref,true)
    REGISTERPAPYRUSFUNC(RegisterForExpressionUpdate_Form,true)

    // ----
    #undef REGISTERPAPYRUSFUNC
}
