#pragma once

namespace UD
{
    inline const auto SaveSerData = _byteswap_ulong('UDSV');

    class SaveManager
    {
    SINGLETONHEADER(SaveManager)
    public:
        void Reload();


        void SetSave(string a_serdata, bool a_decode);
        string GetSaveString(bool a_encode);


        iptree GetSave(){ return _save; }

        // Internal API
        string GetValue(string a_key,string a_defvalue);
        void SetValue(string a_key,string a_value);

        void OnGameLoaded(SKSE::SerializationInterface* serde,uint32_t a_type, uint32_t a_size, uint32_t a_version);
        void OnGameSaved(SKSE::SerializationInterface* serde);
        void OnRevert(SKSE::SerializationInterface* serde);
    private:
        bool _init = false;
        iptree _save;
        mutable Utils::Spinlock  _lock;
    };

    inline void SetSave(PAPYRUSFUNCHANDLE, string a_input, bool a_devode)
    {
        return SaveManager::GetSingleton()->SetSave(a_input,a_devode);
    }

    inline string GetSave(PAPYRUSFUNCHANDLE, bool a_encode)
    {
        return SaveManager::GetSingleton()->GetSaveString(a_encode);
    }
}