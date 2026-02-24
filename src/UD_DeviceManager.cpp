#include <UD_DeviceManager.h>
#include <UD_Config.h>
//#include <boost/json/src.hpp>
//#include <boost/algorithm/string.hpp>
#include <UD_Utility.h>

SINGLETONBODY(UD::DeviceManager)

void UD::DeviceManager::Reload()
{
    if (!_init || Config::GetSingleton()->GetVariable<bool>("Data.bReloadCache",false))
    {
        _init = true;
        _jsoncache.clear();
        std::string loc_devconfpath = std::filesystem::current_path().string() + "\\Data\\UD\\DeviceConfig";
        std::regex loc_regex(R"regex(.*\\(.*\.[jJ][sS][oO][nN]))regex");
    
        for (const auto & entry : std::filesystem::directory_iterator(loc_devconfpath))
        {
            std::string loc_path = entry.path().string();
    
            if (entry.is_regular_file() && std::regex_match(loc_path,loc_regex)) 
            {
                const std::string loc_jsonname = std::regex_replace(loc_path,loc_regex,"$1");
                std::fstream loc_ifile(loc_path,std::ios::in);
                std::shared_ptr<boost::property_tree::ptree> loc_json = std::shared_ptr<boost::property_tree::ptree>(new boost::property_tree::ptree);
                //boost::json::value loc_json;
                if (loc_ifile.is_open())
                {
                    try
                    {
                        //loc_json = boost::json::parse(loc_ifile);
                        boost::property_tree::read_json(loc_path, *loc_json.get());
                    }
                    catch(const std::exception& e)
                    {
                        ERROR("Error parsing json {} - {}",loc_jsonname,e.what())
                    }
                }
                else
                {
                    ERROR("Could not open file {}",loc_jsonname)
                }
    
    
                std::regex loc_regexname(R"regex((.*\\)(.*)(\.[jJ][sS][oO][nN]))regex");
                const std::string loc_name = std::regex_replace(loc_path,loc_regexname,"$2");
    
                //auto loc_config = std::shared_ptr<DeviceConfigJson>(new DeviceConfigJson{std::shared_ptr<boost::json::value>(new boost::json::value(loc_json)),DeviceConfigStatus::sOK,"OK"});
                auto loc_config = std::shared_ptr<DeviceConfigJson>(new DeviceConfigJson{loc_json,DeviceConfigStatus::sOK,"OK"});
                
                loc_config->InitConfig();
                
                _jsoncache[loc_name] = loc_config;
            }
        }
    
        DEBUG("=== Loaded device config files ===")
        for (auto&& [name,file] : _jsoncache)
        {
            DEBUG("\t{} - {} / {}",name, file->status, file->error)
        }
    }
}

std::string UD::DeviceVariable::GetValue(Object a_device) const
{
    std::string loc_res;
    if (a_device)
    {
        RE::BSScript::Variable* loc_var = nullptr;
        if (property)
        {
            loc_var = a_device->GetProperty(name);
        }
        else
        {
            loc_var = a_device->GetVariable(name);
        }

        if (!loc_var)
        {
            ERROR("Can't find variable {} !",name)
            return "";
        }

        switch (type)
        {
            case DeviceVariableType::eInt:
                loc_res = std::to_string(loc_var->GetSInt());
            break;
            case DeviceVariableType::eFloat:
                loc_res = std::to_string(loc_var->GetFloat());
            break;
            case DeviceVariableType::eBool:
                loc_res = std::to_string((int)loc_var->GetBool());
            break;
            case DeviceVariableType::eString:
                loc_res = loc_var->GetBool();
            break;
            default:
                WARN("Unsupported format {} !",type)
            break;
        }
    }
    return loc_res;
}

void UD::DeviceConfigJson::InitConfig()
{
    if (json)
    {
        try
        {
            config.name = json->get_optional<std::string>("name").get_value_or("MISSINGNAME");
            config.description = json->get_optional<std::string>("description").get_value_or("MISSINGDESC");
            config.script = json->get_optional<std::string>("script").get_value_or("");

            auto loc_variables = json->get_child("variables");

            for(auto&& [path,val] : loc_variables)
            {
                DeviceVariable loc_var;
                loc_var.name        = val.get_optional<std::string>("name").get_value_or("");
                loc_var.nameDocu    = val.get_optional<std::string>("namedoc").get_value_or("MISSINGNAME");
                loc_var.type        = (DeviceVariableType)val.get_optional<int>("type").get_value_or(0);
                loc_var.property    = val.get_optional<bool>("property").get_value_or(true);

                DEBUG("Variable parsed - {} , {} , {} , {}",loc_var.name,loc_var.nameDocu,(int)loc_var.type,loc_var.property)

                config.variables.push_back(loc_var);
            }
        }
        catch(const std::exception& e)
        {
            ERROR("Error initiating device config from json - {}!",e.what())
            return;
        };

        DEBUG("Config initiated")

    }
}
