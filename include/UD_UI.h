#pragma once

namespace UD 
{
    //macro to make the code more readable
    #define PROCESSMETER(x)                     \
    {                                           \
    _mutex.lock();                              \
    for (int i = 0;i< _metersIWW.size();i++)    \
    {                                           \
        if (_metersIWW[i]->id == i_id)          \
        {                                       \
            x                                   \
        }                                       \
    }                                           \
    _mutex.unlock();                            \
    }

    #define PROCESSMETERRET(x,y,z)              \
    {                                           \
    _mutex.lock();                              \
    for (int i = 0;i< _metersIWW.size();i++)    \
    {                                           \
        if (_metersIWW[i]->id == i_id)          \
        {                                       \
            x                                   \
            _mutex.unlock();                    \
            return y;                           \
        }                                       \
    }                                           \
    _mutex.unlock();                            \
    return z;                                   \
    }

    

    class MeterEntryIWW 
    {
    public:
        MeterEntryIWW(std::string s_path, int i_id,std::string s_name,float f_base, float f_rate, bool b_update);
        bool                update      = false;
        const int           id          = -1;
        const std::string   name        = "error";
        float               rate        = 0.0;
        float               value       = 0.0;
        float               mult        = 1.0;

        MeterEntryIWW& operator=(const MeterEntryIWW& source);

        void Process(float f_timemult);
    private:
        const std::string _path;
    };

    class MeterManager
    {
    public:
        inline static void AddEntry(std::string s_path, int i_id, std::string s_name, float f_base, float f_rate, bool b_show)
        {
            _mutex.lock();
            _metersIWW.push_back(std::unique_ptr<MeterEntryIWW>(new MeterEntryIWW(s_path,i_id,s_name,f_base,f_rate,b_show)));
            SKSE::log::info("::AddEntry - id={},update={},rate={},value={}",_metersIWW.back()->id,_metersIWW.back()->update,_metersIWW.back()->rate,_metersIWW.back()->value);
            _mutex.unlock();
        }
        inline static bool RemoveEntry(int i_id)
        {
            _mutex.lock();
            for (int i = 0;i < _metersIWW.size();i++)
            {
                if (_metersIWW[i]->id == i_id) 
                {
                    _metersIWW.erase(_metersIWW.begin() + i);
                    _mutex.unlock();
                    return true;
                }
            }
            _mutex.unlock();
            return false;
        }
        inline static bool RemoveEntry(std::string s_name)
        {
            _mutex.lock();
            for (int i = 0;i< _metersIWW.size();i++)
            {
                if (_metersIWW[i]->name == s_name)
                {
                    _metersIWW.erase(_metersIWW.begin() + i);
                    _mutex.unlock();
                    return true;
                }
            }
            _mutex.unlock();
            return false;
        }
        inline static void Update(float f_timemult = 1.0f)
        {   
            _mutex.lock();
            for (int i = 0;i< _metersIWW.size();i++)
            {
                _metersIWW[i]->Process(f_timemult);
            }
            _mutex.unlock();
        }
        inline static void ToggleMeter(int i_id,bool b_toggle)
        {
            PROCESSMETER(_metersIWW[i]->update = b_toggle;)
        }
        inline static void SetMeterRate(int i_id, float f_newrate)
        {
            SKSE::log::info("::SetMeterRate - id={},rate={}",i_id,f_newrate);
            PROCESSMETER(_metersIWW[i]->rate = f_newrate;)
        }
        inline static void SetMeterMult(int i_id, float f_newmult)
        {
            PROCESSMETER(_metersIWW[i]->mult = f_newmult;)
        }
        inline static void SetMeterValue(int i_id, float f_newvalue)
        {
            SKSE::log::info("::SetMeterValue - id={},value={}",i_id,f_newvalue);
            PROCESSMETER(_metersIWW[i]->value = f_newvalue;)
        }
        inline static float UpdateMeterValue(int i_id, float f_diffvalue)
        {
            PROCESSMETERRET(_metersIWW[i]->value += f_diffvalue;UDTRUNCVALUE(_metersIWW[i]->value,0.0f,100.0f); , _metersIWW[i]->value; , 0.0f;)
        }
        inline static float GetMeterValue(int i_id)
        {
            PROCESSMETERRET( , _metersIWW[i]->value; , 0.0f;)
        }
        inline static int RemoveAll()
        {
            int loc_size = _metersIWW.size();
            _metersIWW.clear();
            return loc_size; 
        }
    public:
        static bool updateallowed;
    protected:
        static std::vector<std::unique_ptr<MeterEntryIWW>>  _metersIWW;
        static std::mutex                  _mutex;
    };


    //Papyrus functions
    inline void AddMeterEntry(PAPYRUSFUNCHANDLE, std::string s_path, int i_id, std::string s_name, float f_base, float f_rate, bool b_show)
    {
        MeterManager::AddEntry(s_path,i_id,s_name,f_base,f_rate,b_show);
    }
    inline void RemoveMeterEntry(PAPYRUSFUNCHANDLE, int i_id)
    {
        MeterManager::RemoveEntry(i_id);
    }
    inline void ToggleAllMeters(PAPYRUSFUNCHANDLE, bool b_toggle)
    {
        MeterManager::updateallowed = b_toggle;
    }
    inline void ToggleMeter(PAPYRUSFUNCHANDLE, int i_id, bool b_toggle)
    {
        MeterManager::ToggleMeter(i_id,b_toggle);
    }
    inline void SetMeterRate(PAPYRUSFUNCHANDLE, int i_id, float f_newrate)
    {
        MeterManager::SetMeterRate(i_id,f_newrate);
    }
    inline void SetMeterMult(PAPYRUSFUNCHANDLE, int i_id, float f_newmult)
    {
        MeterManager::SetMeterMult(i_id,f_newmult);
    }
    inline void SetMeterValue(PAPYRUSFUNCHANDLE, int i_id, float f_newvalue)
    {
        MeterManager::SetMeterValue(i_id,f_newvalue);
    }
    inline float UpdateMeterValue(PAPYRUSFUNCHANDLE, int i_id, float f_diffvalue)
    {
        return MeterManager::UpdateMeterValue(i_id,f_diffvalue);
    }
    inline float GetMeterValue(PAPYRUSFUNCHANDLE,int i_id)
    {
        return MeterManager::GetMeterValue(i_id);
    }
    inline int RemoveAllMeterEntries(PAPYRUSFUNCHANDLE)
    {
        return MeterManager::RemoveAll();
    }
    #undef PROCESSMETER
    #undef PROCESSMETER2
}