#include <UD_Materials.h>
#include <UD_Utility.h>
#include <UD_Config.h>

SINGLETONBODY(UD::MaterialManager)

void UD::MaterialManager::Setup()
{
    if (!_installed)
    {
        LoadKeywords(_steel_words,"asSteelKeywords");
        LoadKeywords(_ebonite_words,"asEboniteKeywords");
        LoadKeywords(_rope_words,"asRopeKeywords");
        LoadKeywords(_secure_words,"asSecureKeywords");
        LoadKeywords(_leather_words,"asLeatherKeywords");
        
        _installed = true;
    }
}

bool UD::MaterialManager::IsSteel(const RE::TESForm* a_id) const
{
    return IsMaterial(a_id,_steel_words);
}

bool UD::MaterialManager::IsEbonite(const RE::TESForm* a_id) const
{
    return IsMaterial(a_id,_ebonite_words);;
}

bool UD::MaterialManager::IsRope(const RE::TESForm* a_id) const
{
    return IsMaterial(a_id,_rope_words);;
}

bool UD::MaterialManager::IsSecure(const RE::TESForm* a_id) const
{
    return IsMaterial(a_id,_secure_words);;
}

bool UD::MaterialManager::IsLeather(const RE::TESForm* a_id) const
{
    return IsMaterial(a_id,_leather_words);;
}

void UD::MaterialManager::LoadKeywords(std::vector<std::string>& a_words, std::string a_cfgkey)
{
    a_words = Config::GetSingleton()->GetArray<std::string>("Materials."+a_cfgkey);
 
    for (auto&& it : a_words) 
    {
        const auto loc_first = it.find_first_not_of(' ');
        const auto loc_last  = it.find_last_not_of(' ');
        it = it.substr(loc_first,loc_last - loc_first + 1);
        std::transform(it.begin(), it.end(), it.begin(), ::tolower);
        LOG("MaterialManager::LoadKeywords({}) - Loaded keyword {}",a_cfgkey,it)
    }
}

bool UD::MaterialManager::IsMaterial(const RE::TESForm* a_id, const std::vector<std::string>& a_words) const
{
    if (a_id == nullptr || a_words.size() == 0 || a_words[0] == "") return false;

    std::string loc_name = a_id->GetName();

    if (loc_name == "") return false;

    std::transform(loc_name.begin(), loc_name.end(), loc_name.begin(), ::tolower);

    for (auto&& it : a_words)
    {
        if (loc_name.find(it) != std::string::npos) return true;
    }
    return false;
}
