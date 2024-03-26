#pragma once

#include <boost/property_tree/ptree.hpp>

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
        std::vector<std::string> GetArrayRaw(std::string a_name, bool a_tolower, std::string a_sep = ",") const;
    };
}