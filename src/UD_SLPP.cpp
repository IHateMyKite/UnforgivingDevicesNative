#include <UD_Config.h>
#include <UD_Macros.h>
#include <UD_SLPP.h>
#include <UD_Utility.h>
#include <UD_Animation.h>
#include <algorithm>
#include <cctype>
#include <iostream>
#include <string>

namespace UD {
    std::string ConvertAnimationSLPP(PAPYRUSFUNCHANDLE, RE::Actor* actor, const std::string originalAnimationName) {
        return ConvertAnimationSLPPNative(actor,originalAnimationName);
    }
    std::string ConvertAnimationSLPPNative(RE::Actor * actor,const std::string originalAnimationName) {
        auto animManager=UD::AnimationManager::GetSingleton();
        
        if (actor) {
            RE::BSAnimationGraphManagerPtr ptr;
            actor->GetAnimationGraphManager(ptr);
            if (ptr) {
                std::string lowerOriginalAnimationName(originalAnimationName);
                std::transform(lowerOriginalAnimationName.begin(), lowerOriginalAnimationName.end(),
                               lowerOriginalAnimationName.begin(), [](char c) { return std::tolower(c); });
                if (animManager->AnimationCacheMap.contains(lowerOriginalAnimationName)) {
                    return animManager->AnimationCacheMap[lowerOriginalAnimationName];
                }
                for (auto graph : ptr->graphs) {
                    if (!graph) {
                        continue;
                    }

                    if (!graph->behaviorGraph) {
                        continue;
                    }
                    if (!graph->behaviorGraph->eventIDMap) {
                        continue;
                    }
                    if (!graph->projectDBData) {
                        continue;
                    }
                        
                    for (int prefix_removal = 0; prefix_removal < 4; prefix_removal += 1) {
                        uint64_t** vtable = (uint64_t**)graph->projectDBData;
                        RE::BSTArray<char*>* array1 = (RE::BSTArray<char*>*)&vtable[0x1a];
                        //SKSE::log::info("vtable {:016X} array {:016X}", (uint64_t)vtable[0], (uint64_t)array);

                        for (auto eventName : *array1) {
                            if (eventName == nullptr) {
                                continue;
                            }
                            std::string lowerEventName(eventName);
                            std::transform(lowerEventName.begin(), lowerEventName.end(), lowerEventName.begin(),
                                           [](char c) { return std::tolower(c); });

                            if (lowerEventName.size() > prefix_removal + 1) {
                                auto found = lowerEventName.find(lowerOriginalAnimationName.substr(prefix_removal));

                                if (found != std::string::npos) {
                                    std::string eventName2(eventName);
                                    if (found + (lowerOriginalAnimationName.size() - prefix_removal) ==
                                        lowerEventName.size()) {
                                        SKSE::log::info("animation {}", eventName2);
                                        animManager->AnimationCacheMap[lowerOriginalAnimationName]=eventName2;
                                        return eventName2;
                                    } else {
                                        SKSE::log::info("incorrect animation {}", eventName2);
                                    }
                                }
                            }
                        }
                    }
                
            
        

 
 

                }
                animManager->AnimationCacheMap[lowerOriginalAnimationName]="ERROR_NOT_FOUND";
            }
            
        }
        return std::string("ERROR_NOT_FOUND");
        
    }
}