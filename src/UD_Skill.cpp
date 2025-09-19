#include <UD_Skill.h>
#include <UD_Utility.h>
#include <UD_Config.h>

namespace UD
{
    SINGLETONBODY(SkillManager)

    typedef RE::PlayerCharacter::PlayerSkills::Data::Skills::Skill Skill;

    int CalculateSkillFromPerks(PAPYRUSFUNCHANDLE, RE::Actor* a_actor, std::string a_skill, int a_increase)
    {
        if (a_actor == nullptr || a_skill == "") return 10;

        LOG("CalculateSkillFromPerks({},{}) - Called",a_actor->GetName(),a_skill)

        auto loc_perks = GetPerksForSkill(NULL,a_skill);

        int loc_res = 0;
        for (auto&& it : loc_perks)
        {
            if (it != nullptr)
            {
                RE::BGSPerk* loc_perk = it->As<RE::BGSPerk>();
                if ((loc_perk != nullptr) && a_actor->HasPerk(loc_perk))
                {
                    loc_res += a_increase;
                }
            }
        }

        //DEBUG("Calculated skill = {} for {}",loc_res,a_skill)

        return loc_res;
    }

    std::vector<RE::BGSPerk*> GetPerksForSkill(PAPYRUSFUNCHANDLE, std::string a_skill)
    {
        RE::ActorValue loc_value = RE::ActorValueList::GetSingleton()->LookupActorValueByName(a_skill);
        auto loc_valueinfo = RE::ActorValueList::GetSingleton()->GetActorValue(loc_value);
        std::vector<RE::BGSPerk*> loc_res;
        if (loc_valueinfo != nullptr && loc_valueinfo->perkTree)
        {
            auto loc_tree = loc_valueinfo->perkTree;
            auto loc_childs = loc_tree->children;
            GetPerksFromTree(loc_res,loc_childs);
        }
        else
        {
            ERROR("GetPerksForSkill({}) : Cant find perk tree")
        }
        return loc_res;
    }

    void AdvanceSkillPerc(PAPYRUSFUNCHANDLE, std::string a_skill, float a_value)
    {
        #define GET_SKILL_DATA() loc_player->GetInfoRuntimeData().skills->data
        RE::PlayerCharacter* loc_player = RE::PlayerCharacter::GetSingleton();
        auto loc_av = GetActorValueByName(a_skill);
        auto loc_avinfo = RE::ActorValueList::GetSingleton()->GetActorValue(loc_av);

        if (loc_avinfo == nullptr) 
        {
            ERROR("AdvanceSkillPerc({},{}) - Can't get actor value info",a_skill,a_value)
            return;
        }

        Skill loc_skill = GetSkillByName(a_skill);
        const float loc_thd = GET_SKILL_DATA()->skills[loc_skill].levelThreshold;

        // Check player level
        const auto  loc_lvl = GET_SKILL_DATA()->skills[loc_skill].level;
        const float loc_val = std::lerp(a_value*0.1,a_value,std::clamp(1.0 - ((loc_lvl - 15)/100.0),0.0,1.0));

        LOG("AdvanceSkillPerc({},{}) - Recalculated perc. = {}, level = {}",a_skill,a_value,loc_val,loc_lvl)

        if (loc_avinfo->skill->useMult)
        {
            const float loc_xp = (loc_thd*loc_val - loc_avinfo->skill->offsetMult)/loc_avinfo->skill->useMult;
            loc_player->AddSkillExperience(loc_av,loc_xp);
        }
        else
        {
            ERROR("AdvanceSkillPerc({},{}) - Use mult. is 0",a_skill,a_value)
        }

        #undef GET_SKILL_DATA
    }

    void GetPerksFromTree(std::vector<RE::BGSPerk*>& a_res,RE::BSTArray<RE::BGSSkillPerkTreeNode*> a_tree)
    {
        for (auto&& it : a_tree)
        {
            if (it != nullptr && it->perk != nullptr)
            {
                if (std::find(a_res.begin(),a_res.end(),it->perk) == a_res.end())
                {
                    a_res.push_back(it->perk);

                    RE::BGSPerk* loc_nextperk = it->perk->nextPerk;
                    do
                    {
                        if (loc_nextperk != nullptr)
                        {
                            a_res.push_back(loc_nextperk);
                            loc_nextperk = loc_nextperk->nextPerk;
                        }
                    }
                    while(loc_nextperk != nullptr);

                    if (it->children.size() > 0)
                    {
                        GetPerksFromTree(a_res,it->children); // Recursion
                    }
                }
            }
        }
    }

    std::unordered_map<std::string,std::pair<Skill,RE::ActorValue>> g_SkillTable = 
    {
        {"OneHanded"   ,{Skill::kOneHanded  , RE::ActorValue::kOneHanded  }},
        {"TwoHanded"   ,{Skill::kTwoHanded  , RE::ActorValue::kTwoHanded  }},
        {"Archery"     ,{Skill::kArchery    , RE::ActorValue::kArchery    }},
        {"Block"       ,{Skill::kBlock      , RE::ActorValue::kBlock      }},
        {"Smithing"    ,{Skill::kSmithing   , RE::ActorValue::kSmithing   }},
        {"HeavyArmor"  ,{Skill::kHeavyArmor , RE::ActorValue::kHeavyArmor }},
        {"LightArmor"  ,{Skill::kLightArmor , RE::ActorValue::kLightArmor }},
        {"Pickpocket"  ,{Skill::kPickpocket , RE::ActorValue::kPickpocket }},
        {"Lockpicking" ,{Skill::kLockpicking, RE::ActorValue::kLockpicking}},
        {"Sneak"       ,{Skill::kSneak      , RE::ActorValue::kSneak      }},
        {"Alchemy"     ,{Skill::kAlchemy    , RE::ActorValue::kAlchemy    }},
        {"Speech"      ,{Skill::kSpeech     , RE::ActorValue::kSpeech     }},
        {"Alteration"  ,{Skill::kAlteration , RE::ActorValue::kAlteration }},
        {"Conjuration" ,{Skill::kConjuration, RE::ActorValue::kConjuration}},
        {"Destruction" ,{Skill::kDestruction, RE::ActorValue::kDestruction}},
        {"Illusion"    ,{Skill::kIllusion   , RE::ActorValue::kIllusion   }},
        {"Restoration" ,{Skill::kRestoration, RE::ActorValue::kRestoration}},
        {"Enchanting"  ,{Skill::kEnchanting , RE::ActorValue::kEnchanting }}
    };

    Skill GetSkillByName(std::string asSkill)
    {
        return g_SkillTable[asSkill].first;
    }

    RE::ActorValue GetActorValueByName(std::string asSkill)
    {
        return g_SkillTable[asSkill].second;
    }
    void SkillManager::Setup()
    {

    }
}