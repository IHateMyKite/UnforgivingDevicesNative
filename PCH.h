#pragma once

#include "RE/Skyrim.h"
#include "SKSE/SKSE.h"
#include <Windows.h>
#undef ERROR
#include "include/UD_H.h"

using namespace std::literals;

namespace UD
{
    typedef uint32_t Handle;

    template <class T>
    using ActorMap = std::unordered_map<Handle, T>;
}