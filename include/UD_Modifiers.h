#pragma once

#include <boost/algorithm/string/predicate.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/lexical_cast.hpp>

namespace UD
{

    bool HasModifier(PAPYRUSFUNCHANDLE,std::vector<std::string> a_mods, std::string a_modname);
    int GetModifierIndex(PAPYRUSFUNCHANDLE,std::vector<std::string> a_mods, std::string a_modname);
    bool ModifierHaveParams(PAPYRUSFUNCHANDLE,std::vector<std::string> a_mods, std::string a_modname);
    std::vector<std::string> getModifierAllParam(PAPYRUSFUNCHANDLE,std::vector<std::string> a_mods, std::string a_modname);
    int GetModifierParamNum(PAPYRUSFUNCHANDLE,std::vector<std::string> a_mods, std::string a_modname);

    inline int _GetModifierIndex(const std::vector<std::string> &a_mods, const std::string &a_modname);
    inline bool _ModifierHaveParams(const std::vector<std::string> &a_mods,const std::string &a_modname);
    inline int _GetModifierParamNum(const std::vector<std::string> &a_mods, const std::string &a_modname);
    template<class T> inline std::vector<T> _getModifierAllParam(std::vector<std::string> a_mods, std::string a_modname);
}