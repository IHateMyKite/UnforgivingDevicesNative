#include <UD_VariableManager.h>

UD::VariableDetails UD::ParseVariable(std::string a_var)
{
    static const std::regex loc_ParseRegex(R"((?:(.+)::(.+)\((\w)*\))*)");
    VariableDetails loc_res;

    loc_res.Owner   = std::regex_replace(a_var, loc_ParseRegex, "$1");
    std::transform(loc_res.Owner.begin(), loc_res.Owner.end(), loc_res.Owner.begin(), ::tolower);
    loc_res.Name    = std::regex_replace(a_var, loc_ParseRegex, "$2");
    const std::string loc_mod = std::regex_replace(a_var, loc_ParseRegex, "$3");
    if (loc_mod != "") loc_res.Mod = ParseVariableMod(loc_mod[0]);
    return loc_res;
}

UD::VariableMod UD::ParseVariableMod(char a_mod)
{
    switch(a_mod)
    {
        case 'R':
            return VariableMod::eRelative;
        break;
        case 'B':
            return VariableMod::eBase;
        break;
        case 'U':
            return VariableMod::eUpdate;
        break;
        case 'D':
            return VariableMod::eDamage;
        break;
        default:
            return VariableMod::eAbsolute;
        break;
    }
}

UD::VariableValue UD::GetVariableRaw(void* a_source, VariableDetails a_var)
{
    //DEBUG("GetVariableRaw(0x{:016X},{},{},{})",(uintptr_t)a_source,a_var.Name,a_var.Owner,a_var.Mod)
    VariableValue loc_res;
    if (a_var.Owner == "thisdevice" && a_source)
    {
        ObjectPtr* loc_device = reinterpret_cast<ObjectPtr*>(a_source);
        Variable* loc_var = nullptr;
        loc_var = loc_device->GetProperty(a_var.Name);
        if (loc_var == nullptr) loc_var = loc_device->GetVariable(a_var.Name);
        if (loc_var == nullptr)
        {
            ERROR("Error getting variable {} from {}",a_var.Name,a_var.Owner)
            return loc_res;
        }

        loc_res = ParsePapVar(loc_var);
    }
    else if (a_var.Owner == "wearer" || a_var.Owner == "helper")
    {
        RE::Actor* loc_actor = reinterpret_cast<RE::Actor*>(a_source);
        const auto loc_av = RE::ActorValueList::GetSingleton()->LookupActorValueByName(a_var.Name);
        const float loc_avbase = loc_actor->AsActorValueOwner()->GetPermanentActorValue(loc_av);
        switch(a_var.Mod)
        {
            case VariableMod::eAbsolute:
                loc_res.Value = std::to_string(loc_actor->AsActorValueOwner()->GetActorValue(loc_av));
            break;
            case VariableMod::eRelative:
                loc_res.Value = std::to_string(loc_actor->AsActorValueOwner()->GetActorValue(loc_av)/loc_avbase);
            break;
            case VariableMod::eBase:
                loc_res.Value = loc_avbase;
            break;
        }
        loc_res.Type  = VariableType::kFloat;
    }
    else
    {
        ERROR("Unsupported variable owner {}",a_var.Owner)
        loc_res.Type = VariableType::kNone;
    }
    
    return loc_res;
}

UD::VariableValue UD::SetVariableRaw(void* a_source, VariableDetails a_var, VariableValue& a_val)
{
    UD::VariableValue loc_res;
    //DEBUG("SetVariableRaw(0x{:016X},{},{},{},{})",(uintptr_t)a_source,a_var.Name,a_var.Owner,a_var.Mod,a_val.Value)
    if (a_var.Owner == "thisdevice" && a_source)
    {
        ObjectPtr* loc_device = reinterpret_cast<ObjectPtr*>(a_source);
        Variable* loc_var = nullptr;
        loc_var = loc_device->GetProperty(a_var.Name);
        if (loc_var == nullptr) loc_var = loc_device->GetVariable(a_var.Name);
        if (loc_var == nullptr)
        {
            ERROR("Error getting variable {} from {}",a_var.Name,a_var.Owner)
            return loc_res;
        }
        const VariableType loc_type = loc_var->GetType().GetRawType();
        loc_res.Type = loc_type;
        if (a_var.Mod == VariableMod::eAbsolute)
        {
            switch (loc_type)
            {
                case RE::BSScript::TypeInfo::RawType::kFloat:
                    loc_var->Pack(boost::lexical_cast<float>(a_val.Value));
                    loc_res.Value = a_val.Value;
                break;
                case RE::BSScript::TypeInfo::RawType::kBool:
                    loc_var->Pack(boost::lexical_cast<bool>(a_val.Value));
                    loc_res.Value = a_val.Value;
                break;
                case RE::BSScript::TypeInfo::RawType::kInt:
                    loc_var->Pack(boost::lexical_cast<int>(a_val.Value));
                    loc_res.Value = a_val.Value;
                break;
                case RE::BSScript::TypeInfo::RawType::kString:
                    loc_var->Pack(a_val.Value);
                    loc_res.Value = a_val.Value;
                break;
                default:
                    ERROR("Type of {} currently not supported",a_var.Name)
                    return loc_res;
                break;
            }
        }
        else if (a_var.Mod == VariableMod::eUpdate)
        {
            switch (loc_type)
            {
                case RE::BSScript::TypeInfo::RawType::kFloat:
                {
                    float loc_new = boost::lexical_cast<float>(a_val.Value) + loc_var->Unpack<float>();
                    loc_var->Pack(loc_new);
                    loc_res.Value = std::to_string(loc_new);
                }
                break;
                case RE::BSScript::TypeInfo::RawType::kInt:
                {
                    int loc_new = boost::lexical_cast<int>(a_val.Value) + loc_var->Unpack<int>();
                    loc_var->Pack(loc_new);
                    loc_res.Value = std::to_string(loc_new);
                }
                break;
                default:
                    ERROR("Type of {} currently not supported for update",a_var.Name)
                    return loc_res;
                break;
            }
        }

    }
    else if (a_var.Owner == "wearer" || a_var.Owner == "helper")
    {
        RE::Actor* loc_actor = reinterpret_cast<RE::Actor*>(a_source);
        auto loc_val = boost::lexical_cast<float>(a_val.Value);
        loc_res.Value = a_val.Value;
        loc_res.Type = VariableType::kFloat;
        if (loc_val != 0.0f)
        {
            auto loc_av = RE::ActorValueList::GetSingleton()->LookupActorValueByName(a_var.Name);
            switch(a_var.Mod)
            {
                case VariableMod::eAbsolute:
                    loc_actor->AsActorValueOwner()->SetActorValue(loc_av,loc_val);
                break;
                case VariableMod::eBase:
                    loc_actor->AsActorValueOwner()->SetBaseActorValue(loc_av,loc_val);
                break;
                case VariableMod::eUpdate:
                    loc_actor->AsActorValueOwner()->SetActorValue(loc_av,loc_actor->AsActorValueOwner()->GetActorValue(loc_av) + loc_val);
                break;
                case VariableMod::eDamage:
                    loc_actor->AsActorValueOwner()->RestoreActorValue(RE::ACTOR_VALUE_MODIFIER::kDamage,loc_av,-1.0*loc_val);
                break;
            }
        }
    }
    else
    {
        ERROR("Unsupported variable owner {}",a_var.Owner)
        return loc_res;
    }
    
    return loc_res;
}

UD::VariableValue UD::GetVariableRaw(void* a_source, std::string a_var)
{
    return GetVariableRaw(a_source,ParseVariable(a_var));
}

UD::VariableValue UD::ParsePapVar(Variable* a_var)
{
    VariableValue loc_res;
    const VariableType loc_type = a_var->GetType().GetRawType();
    switch (loc_type)
    {
        case RE::BSScript::TypeInfo::RawType::kFloat:
            loc_res.Value = std::to_string(a_var->GetFloat());
            loc_res.Type  = loc_type;
        break;
        case RE::BSScript::TypeInfo::RawType::kBool:
            loc_res.Value = std::to_string(a_var->GetBool());
            loc_res.Type  = loc_type;
        break;
        case RE::BSScript::TypeInfo::RawType::kInt:
            loc_res.Value = std::to_string(a_var->GetSInt());
            loc_res.Type  = loc_type;
        break;
        case RE::BSScript::TypeInfo::RawType::kString:
            loc_res.Value = a_var->GetString();
            loc_res.Type  = loc_type;
        break;
        default:
            ERROR("Type of {} currently not supported",(int)loc_type)
        break;
    }
    return loc_res;
}
