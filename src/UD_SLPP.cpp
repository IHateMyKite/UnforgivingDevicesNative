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
                    if (!graph) {
                        continue;
                    }
                    
                        if (!graph->characterInstance.setup) {
                            continue;
                        }
                        if (!graph->characterInstance.setup->data) {
                            continue;
                        }
                        if (!graph->characterInstance.setup->data->stringData) {
                            continue;
                        }
                    for (int prefix_removal=0;prefix_removal<4; prefix_removal+=1) {
                        for (auto eventBundle : graph->characterInstance.setup->data->stringData->animationNames) {
                            auto eventName = eventBundle.bundleName;
                            std::string lowerEventName(eventName.c_str());
                            std::transform(lowerEventName.begin(), lowerEventName.end(), lowerEventName.begin(), [] (char c) { return std::tolower(c);});
                            
                            if (lowerEventName.size() > prefix_removal+1) {
                                auto found = lowerEventName.find(lowerOriginalAnimationName.substr(prefix_removal));

                                if (found != std::string::npos) {
                                    std::string eventName2(eventName.c_str());
                                    if (found + (lowerOriginalAnimationName.size()-prefix_removal) == lowerEventName.size()) {
                                        SKSE::log::info("animation {}", eventName2);
                                        return eventName2;
                                    } else {
                                        SKSE::log::info("incorrect animation {}", eventName2);
                                    }
                                }
                                prefix_removal += 1;
                            }
                        }
                    }
                }
            }
        }
        return std::string(originalAnimationName);
    }
}