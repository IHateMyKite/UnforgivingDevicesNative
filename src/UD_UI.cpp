#include <UD_UI.h>

SINGLETONBODY(UD::MeterManager)

namespace UD
{

    //static variables define
    std::vector<std::unique_ptr<MeterEntryIWW>>    MeterManager::_metersIWW        = std::vector<std::unique_ptr<MeterEntryIWW>>();
    std::vector<std::unique_ptr<MeterEntrySkyUi>>  MeterManager::_metersSkyUi      = std::vector<std::unique_ptr<MeterEntrySkyUi>>();
    bool                        MeterManager::updateallowed     = true;

    MeterEntryIWW::MeterEntryIWW(std::string s_path, int i_id,std::string s_name, Formula a_formula, float f_base, float f_rate, bool b_update) :  path(s_path), id(i_id), name(s_name)
    {
        value   = f_base;
        rate    = f_rate;
        update  = b_update;
        formula = a_formula;
        extclass = nullptr;
    }

    MeterEntrySkyUi::MeterEntrySkyUi(std::string s_path,std::string s_name, Formula a_formula,float f_base, float f_rate, bool b_update) :  MeterEntryIWW(s_path,-1,s_name,a_formula,f_base,f_rate,b_update)
    {
    }

    inline MeterEntryIWW& MeterEntryIWW::operator=(const MeterEntryIWW& source)
    {
        if (this == &source) return *this;
        *this = MeterEntryIWW(source.path, source.id, source.name,source.formula, source.value,source.rate,source.update);
        this->mult = source.mult;
        return *this;
    }

    void MeterEntryIWW::Process(const float& f_timemult)
    {
        if (newvalueset && frameskip <= 0)
        {
            value = newvalue;
            newvalueset = false;
        }
        if (update)
        {
            if (frameskip <= 0)
            {
                if (extcalc == nullptr || extclass == nullptr)
                {
                    switch (formula)
                    {
                    case tLin:
                        value += UDCONVERTMULT*((rate/60.0f)*mult*f_timemult);
                        if      (value > 100.0f)    value = 100.0f;
                        else if (value < 0.0f)      value = 0.0f;
                        break;
                    case tLinRepeat:
                        value += UDCONVERTMULT*((rate/60.0f)*mult*f_timemult);
                        if      (value >=   100.0f) 
                        {
                            value       =   100.0f;
                            frameskip   =   FRAMESKIP_LINREPEAT;
                        }
                        else if (value <=   0.0f)
                        {
                            value       =   0.0f;
                            frameskip   =   FRAMESKIP_LINREPEAT;
                        }
                        break;
                    }
                }
                else
                {
                    value = extcalc(*extclass);
                }
            }
            else 
            {
                frameskip--;
                if (frameskip <= 0)
                {
                    switch (formula)
                    {
                    case tLinRepeat:
                        if (value == 100.0f) 
                        {
                            value =  0.0f;
                        }
                        else if (value == 0.0f)
                        {
                            value =  100.0f;
                        }
                        break;
                    }
                }
            }

            RE::UI* loc_ui = nullptr;
            if (loc_ui == nullptr) loc_ui = RE::UI::GetSingleton();
            if (loc_ui == nullptr) return;
            //get hud view
            RE::GPtr<RE::IMenu> loc_hudmenu = nullptr;
            if (loc_hudmenu.get() == nullptr) loc_hudmenu = loc_ui->menuMap.find("HUD Menu")->second.menu;
            if (loc_hudmenu.get() == nullptr) return;
            //get hud movie
            RE::GPtr<RE::GFxMovieView> loc_uimovie = nullptr;
            if (loc_uimovie.get() == nullptr) loc_uimovie = loc_hudmenu->uiMovie;
            if (loc_uimovie.get() == nullptr) return;

            //set path
            std::string loc_path =  path + ".setMeterPercent";

            //set argument
            std::string loc_argstr = std::string(std::to_string(id) + "|" + std::to_string(value));
            RE::GFxValue loc_arg;
            loc_arg.SetString(loc_argstr);

            loc_ui->processMessagesLock.Lock();

            //invoke action script function
            loc_uimovie->Invoke(loc_path.c_str(),NULL,&loc_arg,1);

            loc_uimovie->Advance(0.0f);

            loc_ui->processMessagesLock.Unlock();
        }
    }

    void MeterEntrySkyUi::Process(const float& f_timemult)
    {
        if (newvalueset && frameskip <= 0)
        {
            value = newvalue;
            newvalueset = false;
        }
        if (update)
        {
            if (frameskip <= 0)
            {
                if (extcalc == nullptr || extclass == nullptr)
                {
                    switch (formula)
                    {
                    case tLin:
                        value += UDCONVERTMULT*((rate/60.0f)*mult*f_timemult);
                        if      (value > 100.0f)    value = 100.0f;
                        else if (value < 0.0f)      value = 0.0f;
                        break;
                    case tLinRepeat:
                        value += UDCONVERTMULT*((rate/60.0f)*mult*f_timemult);
                        if      (value >=   100.0f) 
                        {
                            value       =   100.0f;
                            frameskip   =   FRAMESKIP_LINREPEAT;
                        }
                        else if (value <=   0.0f)
                        {
                            value       =   0.0f;
                            frameskip   =   FRAMESKIP_LINREPEAT;
                        }
                        break;
                    }
                }
                else
                {
                    value = extcalc(*extclass);
                }
            }
            else
            {
                frameskip--;
                if (frameskip <= 0) //frame skip end
                {
                    switch (formula)
                    {
                    case tLinRepeat:
                        if (value == 100.0f) 
                        {
                            value =  0.0f;
                        }
                        else if (value == 0.0f)
                        {
                            value =  100.0f;
                        }
                        break;
                    }
                }
            }

            RE::UI* loc_ui = nullptr;
            if (loc_ui == nullptr) loc_ui = RE::UI::GetSingleton();
            if (loc_ui == nullptr) return;
            //get hud view
            RE::GPtr<RE::IMenu> loc_hudmenu = nullptr;
            if (loc_hudmenu.get() == nullptr) loc_hudmenu = loc_ui->menuMap.find("HUD Menu")->second.menu;
            if (loc_hudmenu.get() == nullptr) return;
            //get hud movie
            RE::GPtr<RE::GFxMovieView> loc_uimovie = nullptr;
            if (loc_uimovie.get() == nullptr) loc_uimovie = loc_hudmenu->uiMovie;
            if (loc_uimovie.get() == nullptr) return;

            //set path
            std::string loc_path =  path + ".setPercent";

            //set argument
            RE::GFxValue loc_arg[2];
            loc_arg[0].SetNumber(value/100.0f);
            loc_arg[1].SetNumber(UDSKYUIFORCE);

            loc_ui->processMessagesLock.Lock();

            //invoke action script function
            loc_uimovie->Invoke(loc_path.c_str(),NULL,loc_arg,2);

            loc_uimovie->Advance(0.0f);

            loc_ui->processMessagesLock.Unlock();
        }
    }

}