#pragma once

namespace UD
{
    typedef RE::PlayerCharacter::PlayerSkills::Data::Skills::Skill Skill;

    int CalculateSkillFromPerks(PAPYRUSFUNCHANDLE, RE::Actor* a_actor, std::string a_skill, int a_increase);
    std::vector<RE::BGSPerk*> GetPerksForSkill(PAPYRUSFUNCHANDLE,std::string a_skill);
    void AdvanceSkillPerc(PAPYRUSFUNCHANDLE,std::string a_skill,float a_value);
    void GetPerksFromTree(std::vector<RE::BGSPerk*>& a_res,RE::BSTArray<RE::BGSSkillPerkTreeNode*> a_tree);

    Skill GetSkillByName(std::string asSkill);
    RE::ActorValue GetActorValueByName(std::string asSkill);

    class SkillManager
    {
    SINGLETONHEADER(SkillManager)
    public:
        void Setup();
    };
}