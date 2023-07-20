#include <UD_Modifiers.h>


namespace UD 
{
    bool HasModifier(PAPYRUSFUNCHANDLE, std::vector<std::string> a_mods, std::string a_mod)
    {
        UDSKSELOG("HasModifier called")
        return (_GetModifierIndex(a_mods,a_mod) != -1);
    }

    int GetModifierIndex(PAPYRUSFUNCHANDLE, std::vector<std::string> a_mods, std::string a_mod)
    {
        UDSKSELOG("GetModifierIndex called")
        return _GetModifierIndex(a_mods,a_mod);
    }

    bool ModifierHaveParams(PAPYRUSFUNCHANDLE, std::vector<std::string> a_mods, std::string a_mod)
    {
        UDSKSELOG("ModifierHaveParams called")
        return _ModifierHaveParams(a_mods,a_mod);
    }

    std::vector<std::string> getModifierAllParam(PAPYRUSFUNCHANDLE, std::vector<std::string> a_mods, std::string a_mod)
    {
        UDSKSELOG("getModifierAllParam called")
        return _getModifierAllParam<std::string>(a_mods,a_mod);
    }

    int GetModifierParamNum(PAPYRUSFUNCHANDLE, std::vector<std::string> a_mods, std::string a_modname)
    {
        UDSKSELOG("GetModifierParamNum called")
        return _GetModifierParamNum(a_mods,a_modname);
    }

    int _GetModifierIndex(const std::vector<std::string> &a_mods, const std::string &a_mod)
    {
        for(int i = 0; i < a_mods.size(); i++)
        {
            size_t loc_commpos = a_mods[i].find_first_of(';');
            if (loc_commpos != std::string::npos)
            {
                if (boost::iequals(a_mods[i].substr(0,loc_commpos),a_mod))  
                {
                    return i;
                }
            }
            else if (boost::iequals(a_mods[i],a_mod))
            {
                return i;
            }
        }
        return -1;
    }
    bool _ModifierHaveParams(const std::vector<std::string>& a_mods, const std::string& a_mod)
    {
        int loc_id = _GetModifierIndex(a_mods, a_mod);
        if (loc_id != -1)
        {
            std::string loc_mod = a_mods[loc_id];
            if (loc_mod.find_first_of(';')) return true;
            else return false;

        }
        else return false;
    }
    int _GetModifierParamNum(const std::vector<std::string>& a_mods, const std::string& a_modname)
    {
        std::vector<std::string> loc_params = _getModifierAllParam<std::string>(a_mods,a_modname);
        return static_cast<int>(loc_params.size());
    }

    template<class T>
    std::vector<T> _getModifierAllParam(std::vector<std::string> a_mods, std::string a_modname)
    {
        int loc_id = _GetModifierIndex(a_mods, a_modname);
        std::string loc_mod = a_mods[loc_id];
        if ((loc_id != -1) && (loc_mod.find_first_of(';') != std::string::npos))
        {
            //separate to header and parameters
            std::vector<std::string> loc_modifier; 
            boost::split(loc_modifier,loc_mod,boost::is_any_of(";"));
    
            //separete parameters
            std::vector<std::string> loc_params;
            boost::split(loc_params,loc_modifier[1],boost::is_any_of(","));
    
            std::vector<T> loc_res;
            for (auto&& it : loc_params)
            {
                loc_res.push_back(boost::lexical_cast<T>(it));
            }
            return loc_res;
        }
        else
        {
            return std::vector<T>();
        }
    }
}