#pragma once

//copied from DD
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/ini_parser.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/lexical_cast.hpp>

namespace UD
{
    class Config
    {
    public:
        Config(Config &) = delete;
        void operator=(const Config &) = delete;
        static Config* GetSingleton();
    public:
        void Setup();
        template<typename T> T GetVariable(std::string a_name, T a_def) const;
        template<typename T> std::vector<T> GetArray(std::string a_name, std::string a_sep = ",") const;
    protected:
        Config(std::string a_filename) : _filename(a_filename) {}
        ~Config(){}
        static Config* _this;
        const std::string _filename;
    private:
        bool _ready = false;
        boost::property_tree::ptree _config;
        mutable std::unordered_map<std::string,void*> _catche;
        std::vector<std::string> GetArrayRaw(std::string a_name, std::string a_sep = ",") const;
    };
}