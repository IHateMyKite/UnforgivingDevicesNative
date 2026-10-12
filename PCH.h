#pragma once

#include "RE/Skyrim.h"
#include "SKSE/SKSE.h"
#include <Windows.h>
#undef ERROR
#include "include/UD_H.h"
#include <UD_Spinlock.h>
#include <boost/json.hpp>
#include <boost/algorithm/clamp.hpp>
#include <boost/math/special_functions/round.hpp>
#include <boost/algorithm/string/predicate.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/ini_parser.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <boost/archive/iterators/base64_from_binary.hpp>
#include <boost/archive/iterators/binary_from_base64.hpp>
#include <boost/archive/iterators/insert_linebreaks.hpp>
#include <boost/archive/iterators/transform_width.hpp>
#include <boost/archive/iterators/ostream_iterator.hpp>

using namespace std::literals;
using InternalVM = RE::BSScript::Internal::VirtualMachine;
using Script = RE::BSTTuple<const RE::VMHandle, RE::BSTSmallSharedArray<RE::BSScript::Internal::AttachedScript>>;
using Object = RE::BSTSmartPointer<RE::BSScript::Object>;
using DeviceObj = std::pair<RE::TESObjectARMO*,Object>;
struct DeviceObj2
{
    Object obj;
    RE::TESObjectARMO* id;
    RE::TESObjectARMO* rd;
};
using ObjectPtr = RE::BSScript::Object;
using Variable = RE::BSScript::Variable;
using VariableType = RE::BSScript::TypeInfo::RawType;

template<class T>
struct less_nocase
{
    typedef typename T::value_type Ch;
    std::locale m_locale;
    inline bool operator()(Ch c1, Ch c2) const
    {
        return std::toupper(c1, m_locale) < std::toupper(c2, m_locale);
    }
    inline bool operator()(const T &t1, const T &t2) const
    {
        return std::lexicographical_compare(t1.begin(), t1.end(),
                                            t2.begin(), t2.end(), *this);
    }
};

// Boost
using boost::property_tree::ptree;
using boost::property_tree::iptree;
using boost::property_tree::read_json;
using boost::property_tree::write_json;
using namespace boost::archive::iterators;

using string = std::string;

inline std::string RelToAbsPath(std::string a_relPath)
{
    return std::filesystem::current_path().string() + "\\Data\\" + a_relPath;
}

inline std::string RelToSkyrimPath(std::string a_relPath)
{
    return "\\Data\\" + a_relPath;
}

#define STRKW_HEAVYBONDAGE  "zad_DeviousHeavyBondage"
#define STRKW_YOKE          "zad_DeviousYoke"
#define STRKW_MITTEN        "zad_DeviousBondageMittens"

namespace UD
{
    typedef uint32_t Handle;

    template <class T>
    using ActorMap = std::unordered_map<Handle, T>;
}