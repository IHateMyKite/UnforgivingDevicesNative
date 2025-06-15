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
                    if (!graph->behaviorGraph) {
                        continue;
                    }
                    if (!graph->behaviorGraph->data) {
                        continue;    
                    }
                    if (!graph->behaviorGraph->data->stringData) {
                        continue;
                    }
                    for (auto eventName: graph->behaviorGraph->data->stringData->eventNames) {
                        std::string lowerEventName(eventName.c_str());
                        std::transform(lowerEventName.begin(), lowerEventName.end(), lowerEventName.begin(), [] (char c) { return std::tolower(c);});
                        
                        auto found=lowerEventName.find(lowerOriginalAnimationName);
                        
                        if (found != std::string::npos) {
                            std::string eventName2(eventName.c_str());
                            if (found+lowerOriginalAnimationName.size() == lowerEventName.size())
                            {
                                
                                SKSE::log::info("animation {}",eventName2);
                                return eventName2;
                            } else {
                                SKSE::log::info("incorrect animation {}",eventName2);
                            }
                        }
                    }
                }
            }
        }
        return std::string(originalAnimationName);
    }
}