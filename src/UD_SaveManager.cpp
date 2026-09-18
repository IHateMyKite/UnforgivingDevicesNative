#include "UD_SaveManager.h"
#include <UD_Config.h>
#include <UD_Utility.h>

SINGLETONBODY(UD::SaveManager)

using boost::property_tree::ptree;
using boost::property_tree::read_json;
using boost::property_tree::write_json;

void UD::SaveManager::Reload()
{
    if (!_init || Config::GetSingleton()->GetVariable<bool>("Data.bReloadCache",false))
    {
        _init = true;
        DEBUG("Save = {}",GetSaveString(false))
        DEBUG("Save(Base64) = {}",GetSaveString(true))
    }
}

// https://stackoverflow.com/questions/12394472/serializing-and-deserializing-json-with-boost

void UD::SaveManager::SetSave(string a_serdata,bool a_decode)
{
    if (a_decode)
    {
        typedef transform_width<binary_from_base64<const char *>,8,6> base64_dec;

        std::stringstream os;
        std::copy(
            base64_dec(a_serdata.c_str()),
            base64_dec(a_serdata.c_str() + a_serdata.size()),
            ostream_iterator<char>(os)
        );
        a_serdata = os.str();
    }

    _save = Utility::DeserializeJson(a_serdata);

    DEBUG("SetSave({},{}) called -> {}",a_serdata,a_decode,Utility::SerializeJson(_save))
}

string UD::SaveManager::GetSaveString(bool a_encode)
{
    string loc_res = "";
    loc_res = Utility::SerializeJson(_save);

    if (a_encode)
    {

        typedef base64_from_binary<transform_width<const char *,6,8>> base64_enc;

        string loc_raw = loc_res;
        std::stringstream os;
        std::copy(
            base64_enc(loc_raw.c_str()),
            base64_enc(loc_raw.c_str() + loc_raw.size()),
            ostream_iterator<char>(os)
        );
        return os.str();
    }

    return loc_res;
}

string UD::SaveManager::GetValue(string a_key, string a_defvalue)
{
    auto loc_res = _save.get_optional<string>(a_key).get_value_or(a_defvalue);;
    //DEBUG("GetValue({},{}) called -> {}",a_key,a_defvalue,loc_res)
    return loc_res;
}

void UD::SaveManager::SetValue(string a_key, string a_value)
{
    DEBUG("SetValue({},{}) called",a_key,a_value)
    _save.put(a_key,a_value);
}

void UD::SaveManager::OnGameLoaded(SKSE::SerializationInterface* serde, uint32_t a_type, uint32_t a_size, uint32_t a_version)
{
    
    Utils::UniqueLock lock(_lock);

    if (a_type == SaveSerData)
    {
        DEBUG("Reading saved Save data")
        uint32_t loc_readdata = 0U;

        std::unique_ptr<char> loc_buffer = std::unique_ptr<char>(new char[65535U]);
        memset(loc_buffer.get(),0,65535U);

        loc_readdata += serde->ReadRecordData(loc_buffer.get(), a_size);

        string loc_str = loc_buffer.get();

        DEBUG("Save data read. Total Number of bytes read = {}",loc_readdata)
        DEBUG("Save data read. Runtime Data = {}",loc_str)

        SetSave(loc_str,false);

        DEBUG("Save data read. Save = {}",Utility::SerializeJson(_save))
    }
}

void UD::SaveManager::OnGameSaved(SKSE::SerializationInterface* serde)
{
    Utils::UniqueLock lock(_lock);

    if (!serde->OpenRecord(SaveSerData, 0))
    {
        ERROR("Failed to open save record for Save data")
        return;
    }
    string loc_str = GetSaveString(false);

    if (!serde->WriteRecordData(loc_str.c_str(),loc_str.size()))
    {
        ERROR("Failed to save the Save data to cosave")
    }
}

void UD::SaveManager::OnRevert(SKSE::SerializationInterface* serde)
{
    Utils::UniqueLock lock(_lock);
    _save.clear();
}
