#include <UD_Utility.h>
namespace UD 
{
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

    bool iInRange(PAPYRUSFUNCHANDLE, int a_value, int a_min, int a_max)
    {
        return (a_value >= a_min && a_value <= a_max);
    }

    bool fInRange(PAPYRUSFUNCHANDLE, float a_value, float a_min, float a_max)
    {
        return (a_value >= a_min && a_value <= a_max);
    }

    std::string FormatFloat(PAPYRUSFUNCHANDLE, float a_value, int a_floatpoints)
    {
        if (a_floatpoints <= 0) return std::to_string(static_cast<int>(a_value));

        std::string loc_res = std::to_string(a_value);
        const size_t loc_point = loc_res.find('.');

        //only erase if string have dot and offset is valid
        if ((loc_point != loc_res.npos) && ((loc_point + (a_floatpoints + 1)) < loc_res.size()))
        {
            //erase all float points after dot pos + a_floatpoints
            loc_res.erase(loc_point+ (a_floatpoints + 1),loc_res.npos);
        }

        return loc_res;
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

        if (loc_base == nullptr) 
        {
            UDSKSELOG("GetActorName - Actor {:08X} have no actor base!",a_actor->GetFormID())
            return a_actor->GetName();
        }

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

    std::vector<int> DivadeToParts(int a_number, int a_parts)
    {
        const int loc_wholeparts = a_number / a_parts;

        std::vector<int> loc_res(a_parts,loc_wholeparts);

        loc_res.front() += (a_number - loc_wholeparts*a_parts);

        return loc_res;
    }

    bool PluginInstalled(PAPYRUSFUNCHANDLE,std::string a_dll)
    {
        HINSTANCE dllHandle = LoadLibraryA(a_dll.c_str());
        if (dllHandle != NULL)
        {
            return true;
            //FreeLibrary(dllHandle);
        }
        else
        {
            return false;
        }
    }

    SINGLETONBODY(RandomGenerator)

    void RandomGenerator::Setup(CONFIGFILEARG(a_ptree))
    {
        _seed = time(NULL);
    }
    float RandomGenerator::RandomNumber() const
    {
        return static_cast<double>(MWC64X())/_UI32_MAX;
    }
    float RandomGenerator::RandomFloat(const float& a_min, const float& a_max) const
    {
        return a_min + RandomNumber()*(a_max - a_min);
    }
    int RandomGenerator::RandomInt(const int& a_min,const int& a_max) const
    {
        return a_min + boost::math::lround(RandomNumber()*static_cast<float>(a_max - a_min));
    }
    uint32_t RandomGenerator::MWC64X() const
    {
        uint32_t c = (_seed) >> 32, x = (_seed) & 0xFFFFFFFF;
        _seed = x * ((uint64_t)4294883355U) + c;
        return x ^ c;
    }

    SINGLETONBODY(Utility)

    int Utility::CodeBit(int a_codedmap, int a_value, int a_size, int a_index) const
    {
        if (a_index + a_size > 32) return UDBITERRORVALUE;
        int loc_clearmap = (((0x1 << a_size) - 1) << a_index);
        int loc_result = a_codedmap;
        a_value     =  (a_value << a_index) & loc_clearmap;
        loc_result  &= (~loc_clearmap);
        loc_result  |= a_value;
        return loc_result;
    }
}