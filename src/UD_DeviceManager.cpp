#include <UD_DeviceManager.h>
#include <UD_Config.h>
#include <UD_Utility.h>
#include <UD_PapyrusDelegate.h>

SINGLETONBODY(UD::DeviceManager)

void UD::DeviceManager::Reload()
{
    if (!_init || Config::GetSingleton()->GetVariable<bool>("Data.bReloadCache",false))
    {
        _init = true;
        //_jsoncache.clear();
        _DeviceTypes.clear();
        std::string loc_devconfpath = std::filesystem::current_path().string() + "\\Data\\UD\\DeviceConfig";
        std::regex loc_regex(R"regex(.*\\(.*\.[jJ][sS][oO][nN]))regex");
    
        const auto loc_vm = InternalVM::GetSingleton();
        DEBUG("Size: {}",loc_vm->objectTypeMap.size())

        std::string loc_sourcepath1 = std::filesystem::current_path().string() + "\\Data\\Scripts\\Source\\";
        std::string loc_sourcepath2 = std::filesystem::current_path().string() + "\\Data\\Source\\Scripts\\"; // SE bullshit path
        static const std::regex loc_regexDocuStr(R"regex(.*<DOCUSTR\(([\w_]+),(.*)\)>.*)regex");
        static const std::regex loc_regexExport(R"regex(.*<EXPORT\((.*)\)>.*)regex");
        static const std::regex loc_regexAtribute(R"regex(([\w_]+):[ ]*(.+))regex");
        for (auto it : loc_vm->objectTypeMap) 
        {
            if (PapyrusDelegate::GetSingleton()->HaveScriptBase(it.second.get(),"ud_customdevice_renderscript"))
            {
                DEBUG("Script {} is extending UD. Checking for docu",it.first)

                const std::string loc_sourcename = std::string(it.first) + ".psc";
                // Script extends main script. Try to import script file and search it for documentation
                std::fstream loc_sourcefile;
                loc_sourcefile.open(loc_sourcepath1 + loc_sourcename,std::ios::in);
                if (!loc_sourcefile.is_open())
                {
                    // Try other path
                    loc_sourcefile.open(loc_sourcepath2 + loc_sourcename,std::ios::in);
                }

                if (loc_sourcefile.is_open())
                {
                    DeviceConfig loc_config;

                    loc_config.script = std::string(it.first);

                    decltype(loc_config.variables) loc_vars;

                    std::string str; 
                    while (std::getline(loc_sourcefile, str))
                    {
                        boost::trim(str);
                        if (str.contains("<DOCUSTR"))
                        {
                            const std::string loc_key   = std::regex_replace(str,loc_regexDocuStr,"$1");
                            const std::string loc_value = std::regex_replace(str,loc_regexDocuStr,"$2");
                            loc_config.docustr[loc_key] = loc_value;
                            DEBUG("Docustr found {}={} found",loc_key,loc_value)
                            continue;
                        }
                        if (str.contains("<EXPORT("))
                        {
                            DeviceVariable loc_var;
                            
                            std::vector<std::string> loc_parts;
                            boost::split(loc_parts,str,boost::is_any_of(" "),boost::algorithm::token_compress_on);
                             
                            if (loc_parts.size() >= 2)
                            {
                                std::transform(loc_parts[1].begin(),loc_parts[1].end(),loc_parts[1].begin(),::tolower);
                                if (loc_parts[1].contains("property"))
                                {
                                    loc_var.name = loc_parts[2];
                                    DEBUG("Property {} found",loc_var.name)
                                }
                                else
                                {
                                    loc_var.name = loc_parts[1];
                                    DEBUG("Variable {} found",loc_var.name)
                                }
                            }

                            loc_var.atributesRaw = std::regex_replace(str,loc_regexExport,"$1");
                            std::vector<std::string> loc_atributes;
                            boost::split(loc_atributes,loc_var.atributesRaw,boost::is_any_of(","),boost::algorithm::token_compress_on);
                            for (auto atr : loc_atributes)
                            {
                                std::string loc_atrname   = std::regex_replace(atr,loc_regexAtribute,"$1");
                                boost::trim(loc_atrname);
                                std::string loc_atrvalue  = std::regex_replace(atr,loc_regexAtribute,"$2");
                                boost::trim(loc_atrvalue);
                                loc_var.atributes[loc_atrname]  = loc_atrvalue;
                                DEBUG("{} atribute found - {}={}",loc_var.name,loc_atrname,loc_atrvalue)
                            }
                            loc_vars.push_back(loc_var);
                        }
                    }
                    

                    // Order variables by priority
                    std::sort(loc_vars.begin(),loc_vars.end(),[&](DeviceVariable& v1,DeviceVariable& v2) -> bool
                    {
                        auto loc_prio1 = boost::lexical_cast<int>(v1.atributes["prio"].value_or("0"));
                        auto loc_prio2 = boost::lexical_cast<int>(v2.atributes["prio"].value_or("0"));
                        return loc_prio1 > loc_prio2;
                    });


                    loc_config.variables = loc_vars;

                    _DeviceTypes[std::string(it.first)] = loc_config;
                    loc_sourcefile.close();
                }
                else 
                {
                    ERROR("Could not find source file for script {}",loc_sourcename)
                    continue;
                }
            }
        }
    }
}

std::vector<UD::DeviceConfig> UD::DeviceManager::GetDeviceConfigs(Object a_device)
{
    std::vector<std::string> loc_script;

    auto loc_info = a_device->GetTypeInfo();

    while (loc_info)
    {
        loc_script.push_back(loc_info->GetName());
        loc_info = loc_info->GetParent();
    }

    std::vector<DeviceConfig> loc_res(loc_script.size());
    for (auto it = loc_script.rbegin(); it != loc_script.rend(); ++it)
    {
        loc_res.push_back(_DeviceTypes[*it]);
    }
    return loc_res;
}

float UD::DeviceManager::GetDeviceAccessibility(RE::TESObjectARMO* a_rd, ObjectPtr* a_device, RE::Actor* a_actor, RE::Actor* a_helper)
{
    if (!a_rd || !a_device) return 0.0;

    DEBUG("GetDeviceAccessibility called")

    float loc_res = 1.0f;

    if (!a_rd->HasKeywordString(STRKW_HEAVYBONDAGE))
    {
        if (!Utility::ActorFreeHands(a_actor) && !Utility::ActorFreeHands(a_helper))
        {
            loc_res = 0.0f;
        }
        else if (!a_rd->HasKeywordString(STRKW_MITTEN))
        {
            if (Utility::ActorFreeHands(a_actor,true,true))
            {
                loc_res *= 0.5;
            }
            if (Utility::ActorFreeHands(a_helper,true,true))
            {
                loc_res *= 0.5;
            }
        }
    }

    DEBUG("GetDeviceAccessibility -> {}",loc_res)

    // TODO: Add support for hard access
    return std::clamp(loc_res,0.0f,1.0f);
}
