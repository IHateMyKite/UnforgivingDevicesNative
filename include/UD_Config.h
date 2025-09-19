#pragma once

#include <boost/property_tree/ptree.hpp>
#include <UD_Spinlock.h>

//copied from DD
namespace UD
{
    class Config
    {
    SINGLETONHEADER(Config)
    public:
        void Setup();
        template<typename T> T GetVariable(std::string a_name, T a_def) const;
        template<typename T> std::vector<T> GetArray(std::string a_name, std::string a_sep = ",") const;
        std::vector<std::string> GetArrayText(std::string a_name, bool a_lowercase, std::string a_sep = ",") const;
    private:
        bool _ready = false;
        boost::property_tree::ptree _config;
        mutable std::unordered_map<std::string,void*> _catche;
        mutable Utils::Spinlock  _lock;
        std::vector<std::string> GetArrayRaw(std::string a_name, bool a_tolower, std::string a_sep = ",") const;
    };

    int                         GetIniVariableInt(PAPYRUSFUNCHANDLE,std::string a_name, int a_def);
    std::string                 GetIniVariableString(PAPYRUSFUNCHANDLE,std::string a_name, std::string a_def);
    float                       GetIniVariableFloat(PAPYRUSFUNCHANDLE,std::string a_name, float a_def);
    bool                        GetIniVariableBool(PAPYRUSFUNCHANDLE,std::string a_name, bool a_def);
    std::vector<int>            GetIniArrayInt(PAPYRUSFUNCHANDLE,std::string a_name, std::string a_sep);
    std::vector<std::string>    GetIniArrayString(PAPYRUSFUNCHANDLE,std::string a_name, bool a_text, std::string a_sep);
    std::vector<float>          GetIniArrayFloat(PAPYRUSFUNCHANDLE,std::string a_name, std::string a_sep);
    std::vector<bool>           GetIniArrayBool(PAPYRUSFUNCHANDLE,std::string a_name, std::string a_sep);
}