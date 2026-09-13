#pragma once

namespace UD
{
    typedef RE::PlayerCharacter::PlayerSkills::Data::Skills::Skill Skill;

    enum class ConfigStatus : uint8_t
    {
        sOK             = 0U,
        sDisabled       = 1U,
        sMissingMaster  = 2U,
        sError          = 3U
    };

    struct SkyrimSkill
    {
        string  name;
        int     weight;
    };

    struct SkillConfig
    {
        string name;
        string alias;
        string description;
        int         priority;
        float       multiplier;
        std::vector<SkyrimSkill> skills;
    };

    struct SkillConfigJson;
    typedef std::shared_ptr<SkillConfigJson> SkillSetting;
    struct SkillConfigJson
    {
        uint32_t id;
        std::shared_ptr<boost::property_tree::ptree> json;
        ConfigStatus status;
        std::string error;
        SkillConfig config;
    };

    class SkillManager
    {
    SINGLETONHEADER(SkillManager)
    public:
        void Setup();
        void AdvanceSkillPerc(std::string a_skill,float a_value);
        void AdvanceSingleSkillPerc(std::string a_skill,float a_value);
        void GetPerksFromTree(std::vector<RE::BGSPerk*>& a_res,RE::BSTArray<RE::BGSSkillPerkTreeNode*> a_tree);
        Skill GetSkillByName(std::string asSkill);
        RE::ActorValue GetActorValueByName(std::string asSkill);
    private:
        bool InitConfig(SkillSetting a_config);
    private:
        bool _init = false;
        std::unordered_map<std::string,SkillSetting> _skills;
    };


    int CalculateSkillFromPerks(PAPYRUSFUNCHANDLE, RE::Actor* a_actor, std::string a_skill, int a_increase);
    std::vector<RE::BGSPerk*> GetPerksForSkill(PAPYRUSFUNCHANDLE,std::string a_skill);
    inline void AdvanceSkillPerc(PAPYRUSFUNCHANDLE,std::string a_skill,float a_value)
    {
        SkillManager::GetSingleton()->AdvanceSingleSkillPerc(a_skill,a_value);
    }

}