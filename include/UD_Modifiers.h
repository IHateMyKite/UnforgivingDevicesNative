#pragma once

#include <boost/algorithm/string/predicate.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/lexical_cast.hpp>

namespace UD
{
    int                         GetStringParamInt(PAPYRUSFUNCHANDLE, std::string a_DataStr,int a_Index,int a_DefaultValue);
    float                       GetStringParamFloat(PAPYRUSFUNCHANDLE, std::string a_DataStr,int a_Index,float a_DefaultValue);
    std::string                 GetStringParamString(PAPYRUSFUNCHANDLE, std::string a_DataStr,int a_Index,std::string a_DefaultValue);
    std::vector<std::string>    GetModifierAllParam(PAPYRUSFUNCHANDLE,std::string a_param);

    template<class T> inline T _GetModifierParam(const std::string& a_param,int a_Index,T a_DefaultValue);
    template<class T> inline std::vector<T> _GetModifierAllParam(const std::string& a_param);
}