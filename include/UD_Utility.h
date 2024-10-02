#pragma once

namespace UD 
{
    using InternalVM = RE::BSScript::Internal::VirtualMachine;

    #define UDBITERRORVALUE 0xFFFFFFFF

    //copy of RE::InventoryChanges::IItemChangeVisitor, with full definition so it can be inherited from
    class IItemChangeVisitor
    {
    public:
        virtual ~IItemChangeVisitor(){}  // 00

        // add
        virtual RE::BSContainer::ForEachResult Visit(RE::InventoryEntryData* a_entryData) {return RE::BSContainer::ForEachResult::kContinue;}; // 01
        virtual bool ShouldVisit([[maybe_unused]] RE::InventoryEntryData* a_entryData, [[maybe_unused]] RE::TESBoundObject* a_object) { return true; }  // 02
        virtual RE::BSContainer::ForEachResult Unk_03(RE::InventoryEntryData* a_entryData, [[maybe_unused]] void* a_arg2, bool* a_arg3) // 03
        {
            *a_arg3 = true;
            return Visit(a_entryData);
        }

        RE::InventoryChanges::IItemChangeVisitor& AsNativeVisitor(){return *(RE::InventoryChanges::IItemChangeVisitor*)this;}
    };
    static_assert(sizeof(IItemChangeVisitor) == 0x8);

    // Visitor for worn devices
    class WornVisitor : public IItemChangeVisitor
    {
    public:
        WornVisitor(std::function<RE::BSContainer::ForEachResult(RE::InventoryEntryData*)> a_fun) : _fun(a_fun) {};

        virtual RE::BSContainer::ForEachResult Visit(RE::InventoryEntryData* a_entryData) override
        {
            return _fun(a_entryData);
        }
    private:
        std::function<RE::BSContainer::ForEachResult(RE::InventoryEntryData*)> _fun;
    };

    class Utility
    {
    SINGLETONHEADER(Utility)
    public:
        int     CodeBit(int a_codedmap,int a_value,int a_size,int a_index) const;
        int     DecodeBit(int i_codedMap,int i_size,int i_index) const;
        int     Round(float a_value) const;
        int     iRange(int a_value,int a_min,int a_max) const;
        float   fRange(float a_value,float a_min,float a_max) const;
        bool    iInRange(int a_value,int a_min,int a_max) const;
        bool    fInRange(float a_value,float a_min,float a_max) const;

        bool WornHasKeyword(RE::Actor* a_actor, RE::BGSKeyword* a_kw) const;
        bool WornHasKeyword(RE::Actor* a_actor, std::string a_kw) const;
        RE::TESObjectARMO* GetWornArmor(RE::Actor* a_actor,int a_mask) const;
        RE::TESObjectARMO* CheckArmorEquipped(RE::Actor* a_actor, RE::TESObjectARMO* a_device) const;

        bool IsBlockingMenuOpen();
    private:

    };

    inline int CodeBit(PAPYRUSFUNCHANDLE,int a_codedmap,int a_value,int a_size,int a_index)
    {
        return Utility::GetSingleton()->CodeBit(a_codedmap,a_value,a_size,a_index);
    }

    inline int DecodeBit(PAPYRUSFUNCHANDLE,int a_codedMap,int a_size,int a_index)
    {
        return Utility::GetSingleton()->DecodeBit(a_codedMap,a_size,a_index);
    }

    inline int Round(PAPYRUSFUNCHANDLE,float a_value)
    {
        return Utility::GetSingleton()->Round(a_value);
    }
    inline int iRange(PAPYRUSFUNCHANDLE,int a_value,int a_min,int a_max)
    {
        return Utility::GetSingleton()->iRange(a_value,a_min,a_max);
    }
    inline float fRange(PAPYRUSFUNCHANDLE,float a_value,float a_min,float a_max)
    {
        return Utility::GetSingleton()->fRange(a_value,a_min,a_max);
    }
    inline bool iInRange(PAPYRUSFUNCHANDLE,int a_value,int a_min,int a_max)
    {
        return Utility::GetSingleton()->iInRange(a_value,a_min,a_max);
    }
    inline bool fInRange(PAPYRUSFUNCHANDLE,float a_value,float a_min,float a_max)
    {
        return Utility::GetSingleton()->fInRange(a_value,a_min,a_max);
    }

    inline RE::TESObjectARMO* CheckArmorEquipped(PAPYRUSFUNCHANDLE,RE::Actor* a_actor, RE::TESObjectARMO* a_armor)
    {
        return Utility::GetSingleton()->CheckArmorEquipped(a_actor,a_armor);
    }

    std::string FormatFloat(PAPYRUSFUNCHANDLE,float a_value,int a_floatpoints);

    int FloatToInt(PAPYRUSFUNCHANDLE,float a_value);

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

    //replace virtual class method with method a_funptr. 
    //Passes old function in to a_old
    template<class Fun> void HookVirtualMethod(std::array<REL::VariantID, 1> a_vtable,uint16_t a_indxSEAE,uint16_t a_indxVR,uintptr_t a_funptr, REL::Relocation<Fun>& a_old)
    {
        //the fucking what now
        uintptr_t** loc_vtable      = reinterpret_cast<uintptr_t**>(a_vtable[0].address());

        //get function bassed on version of game  (SEAE/VR)
        uintptr_t   loc_funadress   =  reinterpret_cast<uintptr_t>(loc_vtable[REL::Module::IsVR() ? a_indxVR : a_indxSEAE]);

        //save old function
        a_old = loc_funadress;

        //writte data  to vtable
        REL::safe_write(reinterpret_cast<uintptr_t>(&loc_vtable[REL::Module::IsVR() ? a_indxVR : a_indxSEAE]), &a_funptr, sizeof(uintptr_t));
    }

    //replace virtual class method with method a_funptr. 
    //Passes old function in to a_old
    template<class Fun> void HookVirtualMethod(REL::Relocation<uint64_t>& a_vtable,uint16_t a_indxSEAE,uint16_t a_indxVR,uintptr_t a_funptr, REL::Relocation<Fun>& a_old)
    {
        DEBUG("HookVirtualMethod - Adr = 0x{:016X}",a_vtable.address())

        //the fucking what now
        uintptr_t** loc_vtable      = reinterpret_cast<uintptr_t**>(a_vtable.address());

        //get function bassed on version of game  (SEAE/VR)
        uintptr_t   loc_funadress   =  reinterpret_cast<uintptr_t>(loc_vtable[REL::Module::IsVR() ? a_indxVR : a_indxSEAE]);

        //save old function
        a_old = loc_funadress;

        //writte data  to vtable
        REL::safe_write(reinterpret_cast<uintptr_t>(&loc_vtable[REL::Module::IsVR() ? a_indxVR : a_indxSEAE]), &a_funptr, sizeof(uintptr_t));
    }

    //https://cas.ee.ic.ac.uk/people/dt10/research/rngs-gpu-mwc64x.html
    class RandomGenerator
    {
    SINGLETONHEADER(RandomGenerator)
    public:
        void Setup();
        //black magic random generator
        inline float    RandomNumber() const;
        float    RandomFloat(const float& a_min,const float& a_max) const;
        int      RandomInt(const int& a_min,const int& a_max) const;
        int      RandomIdFromDist(const std::vector<int>& a_dist) const;

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
    inline int RandomIdFromDist(PAPYRUSFUNCHANDLE,std::vector<int> a_dist)
    {
        return RandomGenerator::GetSingleton()->RandomIdFromDist(a_dist);
    }

    std::vector<int> DivadeToParts(int a_number, int a_parts);

    bool PluginInstalled(PAPYRUSFUNCHANDLE,std::string a_dll);

    void ToggleDetection(PAPYRUSFUNCHANDLE, bool a_val);
    
    void ForEachReferenceInRange(RE::TESObjectREFR* origin, float radius,
                                 std::function<RE::BSContainer::ForEachResult(RE::TESObjectREFR& ref)> callback);

    template<class T> T GetStringParam(const std::string& a_param,int a_Index,T a_DefaultValue);
    template<class T> std::vector<T> GetStringParamAllInter(const std::string& a_param, const std::string& a_del);

    RE::TESObjectARMO* GetRandomDevice(PAPYRUSFUNCHANDLE,RE::TESLevItem* a_list);

    inline bool IsAnimating(RE::Actor* a_actor)
    {
        static auto loc_datahandler = RE::TESDataHandler::GetSingleton();
        static RE::TESFaction* loc_animationfaction = static_cast<RE::TESFaction*>(loc_datahandler->LookupForm(0x029567,"Devious Devices - Integration.esm"));
        if (a_actor == nullptr || loc_animationfaction == nullptr) return false;

        return a_actor->IsInFaction(loc_animationfaction);
    }

    inline bool ActorIsBound(RE::Actor* a_actor)
    {
        if (a_actor == nullptr) return false;

        return Utility::GetSingleton()->WornHasKeyword(a_actor,"zad_DeviousHeavyBondage");
    }

    inline bool ActorIsBoundCombatDisabled(RE::Actor* a_actor)
    {
        if (a_actor == nullptr) return false;

        return Utility::GetSingleton()->WornHasKeyword(a_actor,"zad_BoundCombatDisableKick");
    }

    bool IsConcentrationSpell(PAPYRUSFUNCHANDLE,RE::SpellItem* a_spell);
    bool IsConcentrationEnch(PAPYRUSFUNCHANDLE,RE::EnchantmentItem* a_ench);

    

}