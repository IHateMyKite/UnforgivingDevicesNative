#pragma once

namespace UD
{
    class MaterialManager
    {
    SINGLETONHEADER(MaterialManager)
    public:
        void Setup(CONFIGFILEARG(a_cfg));

        bool IsSteel(const RE::TESForm* a_id) const;
        bool IsEbonite(const RE::TESForm* a_id) const;
        bool IsRope(const RE::TESForm* a_id) const;
        bool IsSecure(const RE::TESForm* a_id) const;
    private:
        bool _installed = false;
        std::vector<std::string> _steel_words;
        std::vector<std::string> _ebonite_words;
        std::vector<std::string> _rope_words;
        std::vector<std::string> _secure_words;

        void LoadKeywords(std::vector<std::string>& a_words,CONFIGFILEARG(a_cfg),std::string a_cfgkey);
        bool IsMaterial(const RE::TESForm* a_id,const std::vector<std::string>& a_words) const;
    };

    inline bool IsSteel(PAPYRUSFUNCHANDLE,RE::TESObjectARMO* a_form)
    {
        return MaterialManager::GetSingleton()->IsSteel(a_form);
    }
    inline bool IsEbonite(PAPYRUSFUNCHANDLE,RE::TESObjectARMO* a_form)
    {
        return MaterialManager::GetSingleton()->IsEbonite(a_form);
    }
    inline bool IsRope(PAPYRUSFUNCHANDLE,RE::TESObjectARMO* a_form)
    {
        return MaterialManager::GetSingleton()->IsRope(a_form);
    }
    inline bool IsSecure(PAPYRUSFUNCHANDLE,RE::TESObjectARMO* a_form)
    {
        return MaterialManager::GetSingleton()->IsSecure(a_form);
    }
}