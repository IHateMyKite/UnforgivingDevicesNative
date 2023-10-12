#include <UD_UI.h>

SINGLETONBODY(UD::MeterManager)

namespace UD
{
    //static variables define
    std::vector<std::unique_ptr<MeterEntryIWW>>    MeterManager::_metersIWW        = std::vector<std::unique_ptr<MeterEntryIWW>>();
    std::vector<std::unique_ptr<MeterEntrySkyUi>>  MeterManager::_metersSkyUi      = std::vector<std::unique_ptr<MeterEntrySkyUi>>();
    std::mutex                  MeterManager::_mutex            = std::mutex();
    bool                        MeterManager::updateallowed     = true;
    

    MeterEntryIWW::MeterEntryIWW(std::string s_path, int i_id,std::string s_name,float f_base, float f_rate, bool b_update) :  path(s_path), id(i_id), name(s_name)
    {
        value   = f_base;
        rate    = f_rate;
        update  = b_update;
    }

    MeterEntrySkyUi::MeterEntrySkyUi(std::string s_path,std::string s_name,float f_base, float f_rate, bool b_update) :  MeterEntryIWW(s_path,-1,s_name,f_base,f_rate,b_update)
    {
    }

    inline MeterEntryIWW& MeterEntryIWW::operator=(const MeterEntryIWW& source)
    {
        if (this == &source) return *this;
        *this = MeterEntryIWW(source.path, source.id,source.name,source.value,source.rate,source.update);
        this->mult = source.mult;
        return *this;
    }

    void MeterEntryIWW::Process(const float& f_timemult)
    {
        if (update)
        {
            if (extcalc == nullptr)
            {
                value += UDCONVERTMULT*((rate/60.0f)*mult*f_timemult);
                if      (value > 100.0f)    value = 100.0f;
                else if (value < 0.0f)      value = 0.0f;
            }
            else
            {
                value = extcalc(*extclass);
            }
            static RE::UI* loc_ui = RE::UI::GetSingleton();

            //get hud view
            static auto loc_hudmenu = loc_ui->menuMap.find("HUD Menu")->second.menu;
            if (loc_hudmenu == nullptr) return;

            //get hud movie
            static auto loc_uimovie = loc_hudmenu->uiMovie;
            if (loc_uimovie == nullptr) return;

            //set path
            std::string loc_path =  path + ".setMeterPercent";

            //set argument
            std::string loc_argstr = std::string(std::to_string(id) + "|" + std::to_string(value));
            static RE::GFxValue loc_arg;
            loc_arg.SetString(loc_argstr);

            //invoke action script function
            loc_uimovie->Invoke(loc_path.c_str(),NULL,&loc_arg,1);

            loc_uimovie->Advance(0.0f);
        }
    }

    void MeterEntrySkyUi::Process(const float& f_timemult)
    {
        if (update)
        {
            value += UDCONVERTMULT*((rate/60.0f)*mult*f_timemult);
            if      (value > 100.0f)    value = 100.0f;
            else if (value < 0.0f)      value = 0.0f;

            static RE::UI* loc_ui = RE::UI::GetSingleton();

            //get hud view
            static auto loc_hudmenu = loc_ui->menuMap.find("HUD Menu")->second.menu;
            if (loc_hudmenu == nullptr) return;
        
            //get hud movie
            static auto loc_uimovie = loc_hudmenu->uiMovie;
            if (loc_uimovie == nullptr) return;

            //set path
            std::string loc_path =  path + ".setPercent";

            //set argument
            static RE::GFxValue loc_arg[2];
            loc_arg[0].SetNumber(value/100.0f);
            loc_arg[1].SetNumber(UDSKYUIFORCE);

            //invoke action script function
            loc_uimovie->Invoke(loc_path.c_str(),NULL,loc_arg,2);

            loc_uimovie->Advance(0.0f);
        }
    }

}