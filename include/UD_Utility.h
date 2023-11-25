#pragma once

#include <boost/algorithm/clamp.hpp>
#include <boost/math/special_functions/round.hpp>

namespace UD 
{
    #define UDBITERRORVALUE 0xFFFFFFFF

    class Utility
    {
    SINGLETONHEADER(Utility)
    public:
        int CodeBit(int a_codedmap,int a_value,int a_size,int a_index) const;
    };

    inline int CodeBit(PAPYRUSFUNCHANDLE,int a_codedmap,int a_value,int a_size,int a_index)
    {
        return Utility::GetSingleton()->CodeBit(a_codedmap,a_value,a_size,a_index);
    }

    int     DecodeBit(PAPYRUSFUNCHANDLE,int i_codedMap,int i_size,int i_index);
    int     Round(PAPYRUSFUNCHANDLE,float a_value);
    int     iRange(PAPYRUSFUNCHANDLE,int a_value,int a_min,int a_max);
    float   fRange(PAPYRUSFUNCHANDLE,float a_value,float a_min,float a_max);
    bool    iInRange(PAPYRUSFUNCHANDLE,int a_value,int a_min,int a_max);
    bool    fInRange(PAPYRUSFUNCHANDLE,float a_value,float a_min,float a_max);

    std::string FormatFloat(PAPYRUSFUNCHANDLE,float a_value,int a_floatpoints);

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


    //https://cas.ee.ic.ac.uk/people/dt10/research/rngs-gpu-mwc64x.html
    class RandomGenerator
    {
    SINGLETONHEADER(RandomGenerator)
    public:
        void Setup(CONFIGFILEARG(a_ptree));
        //black magic random generator
        inline float    RandomNumber() const;
        float    RandomFloat(const float& a_min,const float& a_max) const;
        int      RandomInt(const int& a_min,const int& a_max) const;

    private:
        inline uint32_t MWC64X() const;
        mutable uint64_t _seed;
    };

    inline float RandomFloat(PAPYRUSFUNCHANDLE,float a_min,float a_max)
    {
        return RandomGenerator::GetSingleton()->RandomFloat(a_min,a_max);
    }
    inline int RandomInt(PAPYRUSFUNCHANDLE,int a_min,int a_max)
    {
        return RandomGenerator::GetSingleton()->RandomInt(a_min,a_max);
    }

    std::vector<int> DivadeToParts(int a_number, int a_parts);

    bool PluginInstalled(PAPYRUSFUNCHANDLE,std::string a_dll);
}