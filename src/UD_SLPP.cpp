#include <UD_Utility.h>
#include <UD_Config.h>
#include <UD_SLPP.h>
#include <UD_Macros.h>
#include <iostream>
#include <string>
#include <algorithm>
#include <cctype>

namespace UD 
{
    std::string ConvertAnimationSLPP(PAPYRUSFUNCHANDLE,RE::Actor * actor,const std::string originalAnimationName) {
        if (actor) {
            RE::BSAnimationGraphManagerPtr ptr;
            actor->GetAnimationGraphManager(ptr);
            if (ptr) {
                std::string lowerOriginalAnimationName(originalAnimationName);
                std::transform(lowerOriginalAnimationName.begin(), lowerOriginalAnimationName.end(), lowerOriginalAnimationName.begin(), [] (char c) { return std::tolower(c);});
                for (auto graph : ptr->graphs) {
                    std::string lowerProjectName(graph->projectName);
                    std::transform(lowerProjectName.begin(), lowerProjectName.end(), lowerProjectName.begin(), [] (char c) { return std::tolower(c);});
                    
                    auto found=lowerProjectName.find(lowerOriginalAnimationName);
                    
                    if (found != std::string::npos) {
                        std::string projectName(graph->projectName);
                        if (found+lowerOriginalAnimationName.size() == lowerProjectName.size())
                        {
                            
                            SKSE::log::info("animation {}",projectName);
                            return std::string(graph->projectName);
                        } else {
                            SKSE::log::info("incorrect animation {}",projectName);
                        }
                    }
                }
            }
        }
        return std::string(originalAnimationName);
    }
}