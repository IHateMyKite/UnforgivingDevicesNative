#include <UD_VariableManager.h>

UD::VariableDetails UD::ParseVariable(std::string a_var)
{
    static const std::regex loc_ParseRegex(R"((?:(.+)::(.+))*)");
    VariableDetails loc_res;

    loc_res.Owner   = std::regex_replace(a_var, loc_ParseRegex, "$1");
    std::transform(loc_res.Owner.begin(), loc_res.Owner.end(), loc_res.Owner.begin(), ::tolower);
    loc_res.Name    = std::regex_replace(a_var, loc_ParseRegex, "$2");

    return loc_res;
}

UD::VariableValue UD::GetVariableRaw(void* a_source, VariableDetails a_var)
{
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
        const VariableType loc_type = loc_var->GetType().GetRawType();
        switch (loc_type)
        {
            case RE::BSScript::TypeInfo::RawType::kFloat:
                loc_res.Value = std::to_string(loc_var->GetFloat());
                loc_res.Type  = loc_type;
            break;
            case RE::BSScript::TypeInfo::RawType::kBool:
                loc_res.Value = std::to_string(loc_var->GetBool());
                loc_res.Type  = loc_type;
            break;
            case RE::BSScript::TypeInfo::RawType::kInt:
                loc_res.Value = std::to_string(loc_var->GetSInt());
                loc_res.Type  = loc_type;
            break;
            case RE::BSScript::TypeInfo::RawType::kString:
                loc_res.Value = loc_var->GetString();
                loc_res.Type  = loc_type;
            break;
            default:
                ERROR("Type of {} currently not supported",a_var.Name)
                return loc_res;
            break;
        }
    }
    else if (a_var.Owner == "wearer")
    {
        /* TODO */
    }
    else
    {
        ERROR("Unsupported variable owner {}",a_var.Owner)
        loc_res.Type = VariableType::kNone;
    }
    
    return loc_res;
}

UD::VariableValue UD::GetVariableRaw(void* a_source, std::string a_var)
{
    return GetVariableRaw(a_source,ParseVariable(a_var));
}
