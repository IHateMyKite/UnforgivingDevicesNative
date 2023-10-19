#include <UD_Utility.h>
namespace UD 
{
    
    int CodeBit(PAPYRUSFUNCHANDLE,int a_codedmap,int a_value,int a_size,int a_index)
    {
        if (a_index + a_size > 32) return UDBITERRORVALUE;
        int loc_clearmap = (((0x1 << a_size) - 1) << a_index);
        int loc_result = a_codedmap;
        a_value     =  (a_value << a_index) & loc_clearmap;
        loc_result  &= (~loc_clearmap);
        loc_result  |= a_value;
        return loc_result;
    }

    int DecodeBit(PAPYRUSFUNCHANDLE,int a_codedmap,int a_size,int a_index)
    {
        a_codedmap >>= a_index;
        a_codedmap &= ((0x00000001 << a_size) - 1);
        return a_codedmap;
    }

    int Round(PAPYRUSFUNCHANDLE, float a_value)
    {
        return boost::math::lround(a_value);
    }

    int iRange(PAPYRUSFUNCHANDLE, int a_value,int a_min,int a_max)
    {
        return boost::algorithm::clamp(a_value,a_min,a_max);
    }

    float fRange(PAPYRUSFUNCHANDLE, float a_value,float a_min,float a_max)
    {
        return boost::algorithm::clamp(a_value,a_min,a_max);
    }

    int FloatToInt(PAPYRUSFUNCHANDLE, float a_value)
    {
        assert (sizeof(int) == sizeof(float));
        return *((int*)&a_value);
    }

    bool IsPlayer(PAPYRUSFUNCHANDLE, RE::Actor* a_actor)
    {
        if (a_actor == nullptr) return false;
        return a_actor->IsPlayerRef();
    }

    std::string GetActorName(PAPYRUSFUNCHANDLE, RE::Actor* a_actor)
    {
        if (a_actor == nullptr) return "ERROR";
        const auto loc_base = a_actor->GetActorBase();
        std::string loc_res = loc_base->GetName();

        if (loc_res == "")
        {
            switch(loc_base->GetSex())
            {
                case RE::SEX::kMale:
                    loc_res = "Unnamed male";
                    break;
                case RE::SEX::kFemale:
                    loc_res = "Unnamed female";
                    break;
                case RE::SEX::kNone:
                    loc_res = "Unnamed person";
                    break;
                default:
                    loc_res = "ERROR";
            }
        }

        return loc_res;
    }
}