#include <UD_Utility.h>
namespace UD 
{
    int Utility::DecodeBit(int a_codedmap,int a_size,int a_index) const
    {
        a_codedmap >>= a_index;
        a_codedmap &= ((0x00000001 << a_size) - 1);
        return a_codedmap;
    }

    int Utility::Round(float a_value) const
    {
        return boost::math::lround(a_value);
    }

    int Utility::iRange(int a_value,int a_min,int a_max) const
    {
        return boost::algorithm::clamp(a_value,a_min,a_max);
    }

    float Utility::fRange(float a_value,float a_min,float a_max) const
    {
        return boost::algorithm::clamp(a_value,a_min,a_max);
    }

    bool Utility::iInRange(int a_value, int a_min, int a_max) const
    {
        return (a_value >= a_min && a_value <= a_max);
    }

    bool Utility::fInRange(float a_value, float a_min, float a_max) const
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
            LOG("GetActorName - Actor {:08X} have no actor base!",a_actor->GetFormID())
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
            FreeLibrary(dllHandle);
            return true;
        }
        else
        {
            return false;
        }
    }

    void ToggleDetection(PAPYRUSFUNCHANDLE, bool a_val)
    {
        if (RE::ProcessLists::GetSingleton() == nullptr) return;
        RE::ProcessLists::GetSingleton()->runDetection = a_val;
    }

    void ForEachReferenceInRange(RE::TESObjectREFR* origin, float radius,
                                 std::function<RE::BSContainer::ForEachResult(RE::TESObjectREFR& ref)> callback) {
        if (origin && radius > 0.0f) {
            const auto originPos = origin->GetPosition();
            auto* tesSingleton = RE::TES::GetSingleton();
            auto* interiorCell = tesSingleton->interiorCell;
            if (interiorCell) {
                interiorCell->ForEachReferenceInRange(originPos, radius,
                                                      [&](RE::TESObjectREFR& a_ref) { return callback(a_ref); });
            } else {
                if (const auto gridLength = tesSingleton->gridCells ? tesSingleton->gridCells->length : 0;
                    gridLength > 0) {
                    const float yPlus = originPos.y + radius;
                    const float yMinus = originPos.y - radius;
                    const float xPlus = originPos.x + radius;
                    const float xMinus = originPos.x - radius;

                    std::uint32_t x = 0;
                    do {
                        std::uint32_t y = 0;
                        do {
                            if (const auto cell = tesSingleton->gridCells->GetCell(x, y); cell && cell->IsAttached()) {
                                if (const auto cellCoords = cell->GetCoordinates(); cellCoords) {
                                    const RE::NiPoint2 worldPos{cellCoords->worldX, cellCoords->worldY};
                                    if (worldPos.x < xPlus && (worldPos.x + 4096.0f) > xMinus && worldPos.y < yPlus &&
                                        (worldPos.y + 4096.0f) > yMinus) {
                                        cell->ForEachReferenceInRange(originPos, radius, [&](RE::TESObjectREFR& a_ref) {
                                            return callback(a_ref);
                                        });
                                    }
                                }
                            }
                            ++y;
                        } while (y < gridLength);
                        ++x;
                    } while (x < gridLength);
                }
            }
        } else {
            RE::TES::GetSingleton()->ForEachReference([&](RE::TESObjectREFR& a_ref) { return callback(a_ref); });
        }
    }

    SINGLETONBODY(RandomGenerator)

    void RandomGenerator::Setup()
    {
        _seed = time(NULL);
    }
    float RandomGenerator::RandomNumber() const
    {
        return static_cast<double>(MWC64X())/_UI32_MAX;
    }
    float RandomGenerator::RandomFloat(const float& a_min, const float& a_max) const
    {
        if (a_min == a_max) return a_min;
        return a_min + RandomNumber()*(a_max - a_min);
    }
    int RandomGenerator::RandomInt(const int& a_min,const int& a_max) const
    {
        if (a_min == a_max) return a_min;
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

    bool Utility::WornHasKeyword(RE::Actor* a_actor, RE::BGSKeyword* a_kw) const
    {
        if ((a_actor == nullptr) || (a_kw == nullptr)) return false;

        //LOG("LibFunctions::WornHasKeyword({},{}) called",a_actor->GetName(),a_kw->GetFormEditorID())

        bool loc_res = false;
        auto loc_visitor = WornVisitor([this,&loc_res,a_kw](RE::InventoryEntryData* a_entry)
        {
            #undef GetObject
            auto loc_object = a_entry->GetObject();
            RE::TESObjectARMO* loc_armor = nullptr;
            if (loc_object != nullptr && loc_object->IsArmor()) loc_armor = static_cast<RE::TESObjectARMO*>(loc_object);

            if (loc_armor != nullptr && loc_armor->HasKeyword(a_kw))
            {
                loc_res = true;
                return RE::BSContainer::ForEachResult::kStop;
            }
            else return RE::BSContainer::ForEachResult::kContinue;
        });
        a_actor->GetInventoryChanges()->VisitWornItems(loc_visitor.AsNativeVisitor());
        return loc_res;
    }

    RE::TESObjectARMO* Utility::GetWornArmor(RE::Actor* a_actor, int a_mask) const
    {
        if (a_actor == nullptr) return nullptr;

        //LOG("LibFunctions::GetWornArmor({},{:08X}) called",a_actor->GetName(),a_mask)

        RE::TESObjectARMO* loc_res = nullptr;
        auto loc_visitor = WornVisitor([this,&loc_res,a_mask](RE::InventoryEntryData* a_entry)
        {
            #undef GetObject
            auto loc_object = a_entry->GetObject();
            RE::TESObjectARMO* loc_armor = nullptr;
            if (loc_object != nullptr && loc_object->IsArmor()) loc_armor = static_cast<RE::TESObjectARMO*>(loc_object);

            if (loc_armor != nullptr && ((int)loc_armor->GetSlotMask() & a_mask))
            {
                loc_res = loc_armor;
                return RE::BSContainer::ForEachResult::kStop;
            }
            else return RE::BSContainer::ForEachResult::kContinue;
        });
        a_actor->GetInventoryChanges()->VisitWornItems(loc_visitor.AsNativeVisitor());
        return loc_res;
    }

    RE::TESObjectARMO* Utility::CheckArmorEquipped(RE::Actor* a_actor, RE::TESObjectARMO* a_armor) const
    {
        if (a_actor == nullptr || a_armor == nullptr) return nullptr;
        
        uint32_t loc_mask = 0x00000001;
        const uint32_t loc_devicemask = (uint32_t)a_armor->GetSlotMask();

        for (loc_mask = 0x00000001; loc_mask < loc_devicemask; loc_mask <<= 1)
        {
            if (loc_mask > loc_devicemask) break;

            if (loc_mask & loc_devicemask)
            {
                RE::TESObjectARMO* loc_armor = GetWornArmor(a_actor,loc_mask);
                if (loc_armor != nullptr)
                {
                    if (loc_armor == a_armor) return nullptr;
                    else
                    {
                        return loc_armor;
                    }
                }
            }
        }
        return nullptr;
    }

    int GetStringParamInt(PAPYRUSFUNCHANDLE, std::string a_DataStr, int a_Index, int a_DefaultValue)
    {
        //LOG("GetStringParamInt({},{},{})",a_DataStr,a_Index,a_DefaultValue)
        return GetStringParam<int>(a_DataStr,a_Index,a_DefaultValue);
    }

    float GetStringParamFloat(PAPYRUSFUNCHANDLE, std::string a_DataStr, int a_Index, float a_DefaultValue)
    {
        //LOG("GetStringParamFloat({},{},{})",a_DataStr,a_Index,a_DefaultValue)
        return GetStringParam<float>(a_DataStr,a_Index,a_DefaultValue);
    }

    std::string GetStringParamString(PAPYRUSFUNCHANDLE, std::string a_DataStr, int a_Index, std::string a_DefaultValue)
    {
        //LOG("GetStringParamString({},{},{})",a_DataStr,a_Index,a_DefaultValue)
        return GetStringParam<std::string>(a_DataStr,a_Index,a_DefaultValue);
    }

    std::vector<std::string> GetStringParamAll(PAPYRUSFUNCHANDLE, std::string a_param)
    {
        //LOG("GetModifierAllParam({})",a_param)
        return GetStringParamAllInter<std::string>(a_param,",");
    }

    template<class T>
    T GetStringParam(const std::string& a_param, int a_Index, T a_DefaultValue)
    {
        const std::vector<std::string> loc_para = GetStringParamAllInter<std::string>(a_param,",");

        if (a_Index < loc_para.size() && a_Index >= 0)
        {
            try
            {
                return ((loc_para[a_Index] != "") ? boost::lexical_cast<T>(loc_para[a_Index]) : a_DefaultValue);
            }
            catch(boost::bad_lexical_cast &)
            {
                LOG("_GetModifierParam({},{}) - Error casting {} to {}",a_param,a_Index,a_DefaultValue,a_param,typeid(T).name())
                return a_DefaultValue;
            }
        }
        else
        {
            return a_DefaultValue;
        }
    }

    template<class T>
    std::vector<T> GetStringParamAllInter(const std::string& a_param, const std::string& a_del)
    {
        //separete parameters
        std::vector<std::string> loc_params;
        boost::split(loc_params,a_param,boost::is_any_of(a_del));
    
        std::vector<T> loc_res;
        for (auto&& it : loc_params)
        {
            try
            {
                loc_res.push_back(boost::lexical_cast<T>(it));
            }
            catch(boost::bad_lexical_cast &)
            {
                ERROR("_GetModifierAllParam({}) - Error casting {} to {}",a_param,a_param,typeid(T).name())
                loc_res.push_back(T());
                continue;
            }
        }
        return loc_res;
    }

    template std::string   GetStringParam(const std::string& a_param, int a_Index, std::string a_DefaultValue);
    template int           GetStringParam(const std::string& a_param, int a_Index, int a_DefaultValue);
    template float         GetStringParam(const std::string& a_param, int a_Index, float a_DefaultValue);
    template bool          GetStringParam(const std::string& a_param, int a_Index, bool a_DefaultValue);

    template std::vector<std::string>   GetStringParamAllInter(const std::string& a_param, const std::string& a_del);
    template std::vector<int>           GetStringParamAllInter(const std::string& a_param, const std::string& a_del);
    template std::vector<float>         GetStringParamAllInter(const std::string& a_param, const std::string& a_del);
    template std::vector<bool>          GetStringParamAllInter(const std::string& a_param, const std::string& a_del);
}