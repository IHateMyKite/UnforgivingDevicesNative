#pragma once

#include <boost/algorithm/clamp.hpp>
#include <boost/math/special_functions/round.hpp>

namespace UD 
{
    #define UDBITERRORVALUE 0xFFFFFFFF
    int     CodeBit(PAPYRUSFUNCHANDLE,int i_codedmap,int i_value,int i_size,int i_index);
    int     DecodeBit(PAPYRUSFUNCHANDLE,int i_codedMap,int i_size,int i_index);
    int     Round(PAPYRUSFUNCHANDLE,float a_value);
    int     iRange(PAPYRUSFUNCHANDLE,int a_value,int a_min,int a_max);
    float   fRange(PAPYRUSFUNCHANDLE,float a_value,float a_min,float a_max);

    int     FloatToInt(PAPYRUSFUNCHANDLE,float a_value);

    bool IsPlayer(PAPYRUSFUNCHANDLE,RE::Actor* a_actor);
    std::string GetActorName(PAPYRUSFUNCHANDLE,RE::Actor* a_actor);
    //screw trampolines, we ball
    //should be hopefully compatible with all versions

    //replace virtual class method with method a_funptr. 
    //Passes old function in to a_old
    template<class T, class Fun> void HookVirtualMethod(T* a_this,uint16_t a_indxSEAE,uint16_t a_indxVR,uintptr_t a_funptr, REL::Relocation<Fun>& a_old)
    {
        //the fucking what now
        uintptr_t** loc_vtable      = *reinterpret_cast<uintptr_t ***>(reinterpret_cast<uintptr_t>(a_this));

        //get function bassed on version of game  (SEAE/VR)
        uintptr_t   loc_funadress   =  reinterpret_cast<uintptr_t>(loc_vtable[REL::Module::IsVR() ? a_indxVR : a_indxSEAE]);

        //save old function
        a_old = loc_funadress;

        //writte data  to vtable
        REL::safe_write(reinterpret_cast<uintptr_t>(&loc_vtable[REL::Module::IsVR() ? a_indxVR : a_indxSEAE]), &a_funptr, sizeof(uintptr_t));
    }

    //replace virtual class method with method a_funptr
    //does not return old function. Can be used to unhook method
    template<class T, class Fun> void HookVirtualMethod(T* a_this,uint16_t a_indxSEAE,uint16_t a_indxVR,uintptr_t a_funptr)
    {
        //the fucking what now
        uintptr_t** loc_vtable      = *reinterpret_cast<uintptr_t ***>(reinterpret_cast<uintptr_t>(a_this));

        //get function bassed on version of game  (SEAE/VR)
        uintptr_t   loc_funadress   =  reinterpret_cast<uintptr_t>(loc_vtable[REL::Module::IsVR() ? a_indxVR : a_indxSEAE]);

        //writte data  to vtable
        REL::safe_write(reinterpret_cast<uintptr_t>(&loc_vtable[REL::Module::IsVR() ? a_indxVR : a_indxSEAE]), &a_funptr, sizeof(uintptr_t));
    }
}