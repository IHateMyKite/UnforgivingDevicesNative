#pragma once

#include <OrgasmSystem/OrgasmData.h>

namespace UD 
{
    //macro to make the code more readable
    #define PROCESSMETER(a,x,c)                 \
    {                                           \
    _mutex.lock();                              \
    for (int i = 0;i < a.size();i++)            \
    {                                           \
        if (a[i]->c)                            \
        {                                       \
            x                                   \
        }                                       \
    }                                           \
    _mutex.unlock();                            \
    }

    #define PROCESSMETERRET(a, x, y, z, c)      \
    {                                           \
    _mutex.lock();                              \
    for (int i = 0;i < a.size();i++)            \
    {                                           \
        if (a[i]->c)                            \
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
        const std::string   path;
        MeterEntryIWW& operator=(const MeterEntryIWW& source);
        void Process(const float& f_timemult);

        ORS::OrgasmActorData* extclass;
        std::function<float(const ORS::OrgasmActorData&)> extcalc = nullptr; 
    };

    class MeterEntrySkyUi : public MeterEntryIWW
    {
    public:
        MeterEntrySkyUi(std::string s_path, std::string s_name,float f_base, float f_rate, bool b_update);
        void Process(const float& f_timemult);
    };

    class MeterManager
    {
    SINGLETONHEADER(MeterManager)
    public:
        inline static void Update(const float& f_timemult = 1.0f)
        {   
            _mutex.lock();
            static int i;
            for (i = 0;i < _metersIWW.size();i++)
            {
                _metersIWW[i]->Process(f_timemult);
            }
            for (i = 0;i < _metersSkyUi.size();i++)
            {
                _metersSkyUi[i]->Process(f_timemult);
            }
            _mutex.unlock();
        }
        inline static int RemoveAll()
        {
            _mutex.lock();
            int loc_size1 = _metersIWW.size();
            int loc_size2 = _metersSkyUi.size();
            _metersIWW.clear();
            _metersSkyUi.clear();
            _mutex.unlock();
            return (loc_size1 + loc_size2); 
        }

        //iWantWidget
        inline static bool  IsRegisteredIWW(int i_id)
        {
            PROCESSMETERRET(_metersIWW, 
                            , 
                            true ,
                            false,
                            id == i_id)
        }
        inline static void  AddEntryIWW(std::string s_path, int i_id, std::string s_name, float f_base, float f_rate, bool b_show)
        {
            UDSKSELOG("::AddEntryIWW called - path={},i_id={},value={},rate={},show={}",s_path,i_id,f_base,f_rate,b_show);
            if (IsRegisteredIWW(i_id)) 
            {
                SetMeterValueIWW(i_id,f_base);
                SetMeterRateIWW(i_id,f_rate);
                ToggleMeterIWW(i_id,b_show);
                UDSKSELOG("::AddEntryIWW - Entry already exist = updating");
            }
            else
            {
                _mutex.lock();
                _metersIWW.push_back(std::unique_ptr<MeterEntryIWW>(new MeterEntryIWW(s_path,i_id,s_name,f_base,f_rate,b_show)));
                UDSKSELOG("::AddEntryIWW - added id={},update={},rate={},value={}",_metersIWW.back()->id,_metersIWW.back()->update,_metersIWW.back()->rate,_metersIWW.back()->value);
                _mutex.unlock();
            }
        }
        inline static bool  RemoveEntryIWW(int i_id)
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
        inline static void  ToggleMeterIWW(int i_id,bool b_toggle)
        {
            PROCESSMETER(_metersIWW, _metersIWW[i]->update = b_toggle;,id == i_id)
        }
        inline static void  SetMeterRateIWW(int i_id, float f_newrate)
        {
            PROCESSMETER(_metersIWW, _metersIWW[i]->rate = f_newrate;,id == i_id)
        }
        inline static void  SetMeterMultIWW(int i_id, float f_newmult)
        {
            PROCESSMETER(_metersIWW, _metersIWW[i]->mult = f_newmult;,id == i_id)
        }
        inline static void  SetMeterValueIWW(int i_id, float f_newvalue)
        {
            PROCESSMETER(_metersIWW, _metersIWW[i]->value = f_newvalue;,id == i_id)
        }
        inline static float UpdateMeterValueIWW(int i_id, float f_diffvalue)
        {
            PROCESSMETERRET(_metersIWW , 
                            _metersIWW[i]->value += f_diffvalue*_metersIWW[i]->mult;UDTRUNCVALUE(_metersIWW[i]->value,0.0f,100.0f); , 
                            _metersIWW[i]->value ,
                            0.0f , 
                            id == i_id)
        }
        inline static float GetMeterValueIWW(int i_id)
        {
            PROCESSMETERRET(_metersIWW ,
                             , 
                            _metersIWW[i]->value , 
                            0.0f , 
                            id == i_id)
        }

        inline static void  SetExtCalcIWW(int i_id,ORS::OrgasmActorData* a_class,std::function<float(const ORS::OrgasmActorData&)> a_fun)
        {
            PROCESSMETER(_metersIWW, _metersIWW[i]->extcalc = a_fun;_metersIWW[i]->extclass = a_class;,id == i_id)
        }
        inline static void  UnsetExtCalcIWW(int i_id)
        {
            PROCESSMETER(_metersIWW, _metersIWW[i]->extcalc = nullptr;_metersIWW[i]->extclass = nullptr;,id == i_id)
        }
        inline static bool  IsRegisteredSkyUi(const std::string& s_path)  
        {
            PROCESSMETERRET(_metersSkyUi, 
                            , 
                            true ,
                            false,
                            path == s_path)
        }
        inline static void  AddEntrySkyUi(std::string s_path, std::string s_name, float f_base, float f_rate, bool b_show)
        {
            UDSKSELOG("::AddEntrySkyUi called - path={},value={},rate={},update={}",s_path,f_base,f_rate,b_show);
            if (IsRegisteredSkyUi(s_path)) 
            {
                UDSKSELOG("::AddEntrySkyUi - Entry already exist = updating");
                SetMeterValueSkyUi(s_path,f_base);
                SetMeterRateSkyUi(s_path,f_rate);
                ToggleMeterSkyUi(s_path,b_show);
                UDSKSELOG("::AddEntrySkyUi - Entry already exist = updated");
            }
            else
            {
                _mutex.lock();
                _metersSkyUi.push_back(std::unique_ptr<MeterEntrySkyUi>(new MeterEntrySkyUi(s_path,s_name,f_base,f_rate,b_show)));
                UDSKSELOG("::AddEntrySkyUi - added = id={},update={},rate={},value={}",_metersSkyUi.back()->id,_metersSkyUi.back()->update,_metersSkyUi.back()->rate,_metersSkyUi.back()->value);
                _mutex.unlock();
            }
        }
        inline static bool  RemoveEntrySkyUi(const std::string& s_path)
        {
            _mutex.lock();
            for (int i = 0;i < _metersSkyUi.size();i++)
            {
                if (_metersSkyUi[i]->path == s_path) 
                {
                    _metersSkyUi.erase(_metersSkyUi.begin() + i);
                    _mutex.unlock();
                    return true;
                }
            }
            _mutex.unlock();
            return false;
        }
        inline static void  ToggleMeterSkyUi(const std::string& s_path,bool b_toggle)
        {
            PROCESSMETER(_metersSkyUi,_metersSkyUi[i]->update = b_toggle;,path == s_path)
        }
        inline static void  SetMeterRateSkyUi(const std::string& s_path, float f_newrate)
        {
            PROCESSMETER(_metersSkyUi,_metersSkyUi[i]->rate = f_newrate;,path == s_path)
        }
        inline static void  SetMeterMultSkyUi(const std::string& s_path, float f_newmult)
        {
            PROCESSMETER(_metersSkyUi,_metersSkyUi[i]->mult = f_newmult;,path == s_path)
        }
        inline static void  SetMeterValueSkyUi(const std::string& s_path, float f_newvalue)
        {
            PROCESSMETER(_metersSkyUi,_metersSkyUi[i]->value = f_newvalue;,path == s_path)
        }
        inline static float UpdateMeterValueSkyUi(const std::string& s_path, float f_diffvalue)
        {
            PROCESSMETERRET(_metersSkyUi, 
                            _metersSkyUi[i]->value += f_diffvalue*_metersSkyUi[i]->mult;UDTRUNCVALUE(_metersSkyUi[i]->value,0.0f,100.0f); ,
                            _metersSkyUi[i]->value; ,
                            0.0f;,
                            path == s_path)
        }
        inline static float GetMeterValueSkyUi(const std::string& s_path)
        {
            PROCESSMETERRET(_metersSkyUi, 
                            , 
                            _metersSkyUi[i]->value ,
                            0.0f,
                            path == s_path)
        }
    
        inline static void  SetExtCalcSkyUi(const std::string& s_path,ORS::OrgasmActorData* a_class,std::function<float(const ORS::OrgasmActorData&)> a_fun)
        {
            PROCESSMETER(_metersSkyUi,_metersSkyUi[i]->extcalc = a_fun;_metersSkyUi[i]->extclass = a_class;,path == s_path)
        }
        inline static void  UnsetExtCalcSkyUi(const std::string& s_path)
        {
            PROCESSMETER(_metersSkyUi,_metersSkyUi[i]->extcalc = nullptr;_metersSkyUi[i]->extclass = nullptr;,path == s_path)
        }
    public:
        static bool updateallowed;
    protected:
        static std::vector<std::unique_ptr<MeterEntryIWW>>    _metersIWW;
        static std::vector<std::unique_ptr<MeterEntrySkyUi>>  _metersSkyUi;
        static std::mutex                  _mutex;



    };

    //Papyrus functions
    inline void AddMeterEntryIWW(PAPYRUSFUNCHANDLE, std::string s_path, int i_id, std::string s_name, float f_base, float f_rate, bool b_show)
    {
        MeterManager::AddEntryIWW(s_path,i_id,s_name,f_base,f_rate,b_show);
    }
    inline void RemoveMeterEntryIWW(PAPYRUSFUNCHANDLE, int i_id)
    {
        MeterManager::RemoveEntryIWW(i_id);
    }
    inline void ToggleMeterIWW(PAPYRUSFUNCHANDLE, int i_id, bool b_toggle)
    {
        MeterManager::ToggleMeterIWW(i_id,b_toggle);
    }
    inline void SetMeterRateIWW(PAPYRUSFUNCHANDLE, int i_id, float f_newrate)
    {
        MeterManager::SetMeterRateIWW(i_id,f_newrate);
    }
    inline void SetMeterMultIWW(PAPYRUSFUNCHANDLE, int i_id, float f_newmult)
    {
        MeterManager::SetMeterMultIWW(i_id,f_newmult);
    }
    inline void SetMeterValueIWW(PAPYRUSFUNCHANDLE, int i_id, float f_newvalue)
    {
        MeterManager::SetMeterValueIWW(i_id,f_newvalue);
    }
    inline float UpdateMeterValueIWW(PAPYRUSFUNCHANDLE, int i_id, float f_diffvalue)
    {
        return MeterManager::UpdateMeterValueIWW(i_id,f_diffvalue);
    }
    inline float GetMeterValueIWW(PAPYRUSFUNCHANDLE,int i_id)
    {
        return MeterManager::GetMeterValueIWW(i_id);
    }
    
    inline void AddMeterEntrySkyUi(PAPYRUSFUNCHANDLE, std::string s_path, std::string s_name, float f_base, float f_rate, bool b_show)
    {
        MeterManager::AddEntrySkyUi(s_path,s_name,f_base,f_rate,b_show);
    }
    inline void RemoveMeterEntrySkyUi(PAPYRUSFUNCHANDLE, std::string s_path)
    {
        MeterManager::RemoveEntrySkyUi(s_path);
    }
    inline void ToggleMeterSkyUi(PAPYRUSFUNCHANDLE, std::string s_path, bool b_toggle)
    {
        MeterManager::ToggleMeterSkyUi(s_path,b_toggle);
    }
    inline void SetMeterRateSkyUi(PAPYRUSFUNCHANDLE, std::string s_path, float f_newrate)
    {
        MeterManager::SetMeterRateSkyUi(s_path,f_newrate);
    }
    inline void SetMeterMultSkyUi(PAPYRUSFUNCHANDLE, std::string s_path, float f_newmult)
    {
        MeterManager::SetMeterMultSkyUi(s_path,f_newmult);
    }
    inline void SetMeterValueSkyUi(PAPYRUSFUNCHANDLE, std::string s_path, float f_newvalue)
    {
        MeterManager::SetMeterValueSkyUi(s_path,f_newvalue);
    }
    inline float UpdateMeterValueSkyUi(PAPYRUSFUNCHANDLE, std::string s_path, float f_diffvalue)
    {
        return MeterManager::UpdateMeterValueSkyUi(s_path,f_diffvalue);
    }
    inline float GetMeterValueSkyUi(PAPYRUSFUNCHANDLE,std::string s_path)
    {
        return MeterManager::GetMeterValueSkyUi(s_path);
    }

    inline void ToggleAllMeters(PAPYRUSFUNCHANDLE, bool b_toggle)
    {
        MeterManager::updateallowed = b_toggle;
    }
    inline int RemoveAllMeterEntries(PAPYRUSFUNCHANDLE)
    {
        return MeterManager::RemoveAll();
    }
    #undef PROCESSMETER
    #undef PROCESSMETER2
}