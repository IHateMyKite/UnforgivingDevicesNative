#pragma once

namespace UD 
{
    int CodeBit(RE::BSScript::Internal::VirtualMachine* a_vm, const RE::VMStackID a_stackID, RE::StaticFunctionTag*,int i_codedmap,int i_value,int i_size,int i_index);
    int DecodeBit(RE::BSScript::Internal::VirtualMachine* a_vm, const RE::VMStackID a_stackID, RE::StaticFunctionTag*,int i_codedMap,int i_size,int i_index);
}