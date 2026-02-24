#pragma once

namespace UD
{
    struct VariableDetails
    {
        std::string Owner;
        std::string Name;
    };

    struct VariableValue
    {
        std::string     Value = "";
        VariableType    Type = VariableType::kNone;
    };

    VariableDetails ParseVariable(std::string a_var);
    VariableValue   GetVariableRaw(void* a_source,VariableDetails a_var);
    VariableValue   GetVariableRaw(void* a_source,std::string a_var);
    template<class T> T GetValue(VariableValue a_var)
    {
        T loc_res;
        switch (a_var.Type)
        {
            case VariableType::kInt:
            case VariableType::kBool:
            case VariableType::kFloat:
            case VariableType::kString:
            {
                try 
                {
                    loc_res = boost::lexical_cast<T>(a_var.Value);
                }
                catch(const std::exception& e)
                {
                    ERROR("GetValue - Error parsing variable value {} - {}!",a_var.Value,e.what())
                    return T();
                };
            }
            break;
            default:
                ERROR("GetValue - Unsupported type")
            break;
        }
        return loc_res;
    }

    template<class T> T GetVariable(void* a_source,VariableValue a_var)
    {
        T loc_res = GetValue<T>(a_var);
        return loc_res;
    }

    template<class T> T GetVariable(void* a_source,std::string a_var)
    {
        VariableValue loc_var = GetVariableRaw(a_source,a_var);
        return GetVariable<T>(a_source,loc_var);
    }
}