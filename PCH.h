#pragma once

#include "RE/Skyrim.h"
#include "SKSE/SKSE.h"
#include <Windows.h>
#undef ERROR
#include "include/UD_H.h"
#include <boost/json.hpp>
#include <boost/algorithm/clamp.hpp>
#include <boost/math/special_functions/round.hpp>
#include <boost/algorithm/string/predicate.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/ini_parser.hpp>
#include <boost/property_tree/json_parser.hpp>


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

inline std::string RelToAbsPath(std::string a_relPath)
{
    return std::filesystem::current_path().string() + "\\Data\\" + a_relPath;
}

inline std::string RelToSkyrimPath(std::string a_relPath)
{
    return "\\Data\\" + a_relPath;
}

namespace UD
{
    typedef uint32_t Handle;

    template <class T>
    using ActorMap = std::unordered_map<Handle, T>;
}