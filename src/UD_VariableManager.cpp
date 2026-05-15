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

string UD::ProcessDeviceVariable(const VariableValue& a_var, const string& a_format, const string& a_conv)
{
    std::string loc_valueFormated = a_var.Value;
    try
    {
        auto loc_converted = ConvertDeviceVariable(a_var,a_conv);

        switch (loc_converted.Type)
        {
            case VariableType::kBool:
            {
                const auto loc_formval = boost::lexical_cast<bool>(loc_converted.Value);
                loc_valueFormated = std::vformat((a_format),std::make_format_args(loc_formval));
            }
            break;
            case VariableType::kInt:
            {
                const auto loc_formval = boost::lexical_cast<int>(loc_converted.Value);
                loc_valueFormated = std::vformat((a_format),std::make_format_args(loc_formval));
            }
            break;
            case VariableType::kFloat:
            {
                const auto loc_formval = boost::lexical_cast<float>(loc_converted.Value);
                loc_valueFormated = std::vformat((a_format),std::make_format_args(loc_formval));
            }
            break;
            default:
                loc_valueFormated = std::vformat((a_format),std::make_format_args(loc_converted.Value));
            break;
        }
    }
    catch(...)
    {
        ERROR("Error formating {} using {}",a_var.Value,a_format)
    }
    return loc_valueFormated;
}

UD::VariableValue UD::ConvertDeviceVariable(const VariableValue& a_in, string a_conv)
{
    VariableValue loc_res = a_in; // default result is input
    try
    {
        // Check possible convertors
        if (a_conv == "reltoperc") // Relative to percentage
        {
            loc_res.Value = std::to_string(boost::lexical_cast<float>(loc_res.Value)*100.0);
            // Keep type
        }
        else if (a_conv.contains("enum")) // Enum
        {
            static const std::regex loc_enumRegexParams(R"regex(enum\{(.*)\})regex");
            static const std::regex loc_enumValueRegexParams(R"regex((.*)=(.*))regex");
            std::string loc_paramsStr = std::regex_replace(a_conv,loc_enumRegexParams,"$1");
            if (loc_paramsStr != "")
            {
                std::vector<std::string> loc_params;
                boost::split(loc_params,loc_paramsStr,boost::is_any_of(";"),boost::algorithm::token_compress_on);

                for (auto&& it : loc_params)
                {
                    std::string loc_enumvalue = std::regex_replace(it,loc_enumValueRegexParams,"$1");
                    if (loc_res.Value == loc_enumvalue)
                    {
                        std::string loc_enumres = std::regex_replace(it,loc_enumValueRegexParams,"$2");
                        loc_res.Value = loc_enumres;
                        loc_res.Type  = VariableType::kString;
                        break;
                    }
                }
            }
        }
    }
    catch(...)
    {
        ERROR("Error converting value {} using {}",a_in.Value,a_conv)
    }
    return loc_res;
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
        case RE::BSScript::TypeInfo::RawType::kIntArray:
            loc_res.Value = "";
            if (a_var->GetArray())
            {
                auto loc_arr = a_var->GetArray();
                for(size_t i = 0; i < loc_arr->size();i++)
                {
                    loc_res.Value += std::to_string((*loc_arr)[i].GetSInt());
                    if (i != (loc_arr->size() - 1))
                    {
                        loc_res.Value += ",";
                    }
                }
            }
            loc_res.Type  = loc_type;
        break;
        case RE::BSScript::TypeInfo::RawType::kStringArray:
            loc_res.Value = "";
            if (a_var->GetArray())
            {
                auto loc_arr = a_var->GetArray();
                for(size_t i = 0; i < loc_arr->size();i++)
                {
                    loc_res.Value += ((*loc_arr)[i].GetString());
                    if (i != (loc_arr->size() - 1))
                    {
                        loc_res.Value += ",";
                    }
                }
            }
            loc_res.Type  = loc_type;
        break;
        case RE::BSScript::TypeInfo::RawType::kFloatArray:
            loc_res.Value = "";
            if (a_var->GetArray())
            {
                auto loc_arr = a_var->GetArray();
                for(size_t i = 0; i < loc_arr->size();i++)
                {
                    loc_res.Value += std::to_string((*loc_arr)[i].GetFloat());
                    if (i != (loc_arr->size() - 1))
                    {
                        loc_res.Value += ",";
                    }
                }
            }
            loc_res.Type  = loc_type;
        break;
        case RE::BSScript::TypeInfo::RawType::kBoolArray:
            loc_res.Value = "";
            if (a_var->GetArray())
            {
                auto loc_arr = a_var->GetArray();
                for(size_t i = 0; i < loc_arr->size();i++)
                {
                    loc_res.Value += std::to_string((*loc_arr)[i].GetBool());
                    if (i != (loc_arr->size() - 1))
                    {
                        loc_res.Value += ",";
                    }
                }
            }
            loc_res.Type  = loc_type;
        break;
        default:
            //ERROR("Type of {} currently not supported",(int)loc_type)
            loc_res.Value = "";
            loc_res.Type  = loc_type;
        break;
    }
    return loc_res;
}
