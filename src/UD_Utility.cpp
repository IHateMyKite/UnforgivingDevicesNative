#include <UD_Utility.h>
namespace UD 
{
    #define UDBITERRORVALUE 0xFFFFFFFF
    int CodeBit(RE::BSScript::Internal::VirtualMachine* a_vm, const RE::VMStackID a_stackID, RE::StaticFunctionTag*,int i_codedmap,int i_value,int i_size,int i_index)
    {
        if (i_value + i_size > 32) return UDBITERRORVALUE;
        int loc_clearmap = (0x1 << i_size) - 1;
        i_value     &=  loc_clearmap;
        i_value     <<= i_index;
        i_codedmap  &=  loc_clearmap;
        return i_codedmap | i_value;
    }
    int DecodeBit(RE::BSScript::Internal::VirtualMachine* a_vm, const RE::VMStackID a_stackID, RE::StaticFunctionTag*,int i_codedmap,int i_size,int i_index)
    {
        i_codedmap >>= i_index;
        i_codedmap &= (0x1 << i_size) - 1;
        return i_codedmap;
    }
}