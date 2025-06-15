#pragma once
#include <UD_Utility.h>
#include <UD_Config.h>
#include <string>
#include <regex>
namespace UD
{
    std::string ConvertAnimationSLPP(PAPYRUSFUNCHANDLE,RE::Actor * actor,const std::string originalAnimationName);
}