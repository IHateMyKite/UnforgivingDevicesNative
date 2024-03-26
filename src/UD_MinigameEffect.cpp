#include <UD_MinigameEffect.h>
#include <UD_Updater.h>
#include <UD_UI.h>

SINGLETONBODY(UD::MinigameEffectManager)

namespace UD
{
    inline bool _DamageAV(RE::ActorValueOwner* a_avowner,const RE::ActorValue& a_av, const float& f_dmg, const float& f_min)
    {
        if (a_avowner == nullptr) return false;
        if ((f_dmg != 0.0f) && ((a_avowner->GetActorValue(a_av) - f_dmg) > f_min))
        {
            a_avowner->RestoreActorValue(RE::ACTOR_VALUE_MODIFIER::kDamage,a_av,f_dmg);
        }
        return true;
    }

    // Function ProcessAVMinigame(Actor akActor, Float afStamina, Float afHealth, Float afMagicka) global native
    void StartMinigameEffect(PAPYRUSFUNCHANDLE, RE::Actor *a_actor, float f_mult, float f_stamina, float f_health, float f_magicka, bool b_toggle) 
    {
        if (a_actor == nullptr) return;
        //The value in papyrus was not corresponding to real value because of lag. 
        //For that reason, we need to reduce the drain value, otherwise it will be too fast
        //For now, value is reduced by 40%
        float loc_health    = -1.0f*UDCONVERTMULT*f_health/60.0f;
        float loc_stamina   = -1.0f*UDCONVERTMULT*f_stamina/60.0f;
        float loc_magicka   = -1.0f*UDCONVERTMULT*f_magicka/60.0f;
        float loc_mult      = f_mult; 
        float loc_toggle    = b_toggle;

        //LOG("StartMinigameEffect - health={},stamina={},magicka={}",loc_health,loc_stamina,loc_magicka);

        MinigameEffectManager::GetSingleton()->RegisterActor(a_actor,loc_mult,loc_stamina,loc_health,loc_magicka,loc_toggle);
    }

    void EndMinigameEffect(PAPYRUSFUNCHANDLE, RE::Actor *a_actor) 
    {
        //LOG("EndMinigameEffect called")
        MinigameEffectManager::GetSingleton()->RemoveActor(a_actor);
    }

    bool IsMinigameEffectOn(PAPYRUSFUNCHANDLE, RE::Actor *a_actor) 
    {
        //LOG("IsMinigameEffectOn called")
        return MinigameEffectManager::GetSingleton()->started;
    }

    void UpdateMinigameEffectMult(PAPYRUSFUNCHANDLE, RE::Actor *a_actor, float f_mult) 
    {
        //LOG("UpdateMinigameEffectMult called")
        if (MinigameEffectManager::GetSingleton()->IsRegistered(a_actor))
        {
            MinigameEffectManager::GetSingleton()->GetActorControl(a_actor)->mult = f_mult;
        }
    }

    void ToggleMinigameEffect(PAPYRUSFUNCHANDLE, RE::Actor *a_actor, bool b_toggle) 
    {
        //LOG("::ToggleMinigameEffect called {}",b_toggle);
        if (MinigameEffectManager::GetSingleton()->IsRegistered(a_actor))
        {
            MinigameEffectManager::GetSingleton()->GetActorControl(a_actor)->toggle = b_toggle;
        }
    }

    bool MinigameStatsCheck(PAPYRUSFUNCHANDLE, RE::Actor *a_actor, bool a_stamina, bool a_health, bool a_magicka) 
    {
        //LOG("MinigameStatsCheck called")
        if (a_actor != nullptr)
        {
            RE::ActorValueOwner* a_avowner   = a_actor->AsActorValueOwner();
            bool loc_res = true;
            loc_res &= (!a_stamina || (a_avowner->GetActorValue(RE::ActorValue::kStamina) > 0.1));
            loc_res &= (!a_health  || (a_avowner->GetActorValue(RE::ActorValue::kHealth)  > 5.1));
            loc_res &= (!a_magicka || (a_avowner->GetActorValue(RE::ActorValue::kMagicka) > 0.1));
            return loc_res;
        }
        else return false;
    }

    void MinigameEffectSetHealth(PAPYRUSFUNCHANDLE, RE::Actor *a_actor, float f_health)
    {
        //LOG("MinigameEffectSetHealth called")
        float loc_health    = -1.0f*UDCONVERTMULT*f_health/60.0f;
        if (MinigameEffectManager::GetSingleton()->IsRegistered(a_actor))
        {
            MinigameEffectManager::GetSingleton()->GetActorControl(a_actor)->health = loc_health;
        }
    }
    void MinigameEffectSetStamina(PAPYRUSFUNCHANDLE, RE::Actor *a_actor, float f_stamina)
    {
        //LOG("MinigameEffectSetStamina called")
        float loc_stamina    = -1.0f*UDCONVERTMULT*f_stamina/60.0f;
        if (MinigameEffectManager::GetSingleton()->IsRegistered(a_actor))
        {
            MinigameEffectManager::GetSingleton()->GetActorControl(a_actor)->stamina = loc_stamina;
        }
    }
    void MinigameEffectSetMagicka(PAPYRUSFUNCHANDLE, RE::Actor *a_actor, float f_magicka)
    {
        //LOG("MinigameEffectSetMagicka called")
        float loc_magicka    = -1.0f*UDCONVERTMULT*f_magicka/60.0f;
        if (MinigameEffectManager::GetSingleton()->IsRegistered(a_actor))
        {
            MinigameEffectManager::GetSingleton()->GetActorControl(a_actor)->magicka = loc_magicka;
        }
    }

    void MinigameEffectUpdateHealth(PAPYRUSFUNCHANDLE, RE::Actor *a_actor, float f_health)
    {
        //LOG("MinigameEffectUpdateHealth called")
        float loc_health    = -1.0f*UDCONVERTMULT*f_health/60.0f;
        if (MinigameEffectManager::GetSingleton()->IsRegistered(a_actor))
        {
            MinigameEffectManager::GetSingleton()->GetActorControl(a_actor)->health += loc_health;
        }
    }
    void MinigameEffectUpdateStamina(PAPYRUSFUNCHANDLE, RE::Actor *a_actor, float f_stamina)
    {
        //LOG("MinigameEffectUpdateStamina called")
        float loc_stamina    = -1.0f*UDCONVERTMULT*f_stamina/60.0f;
        if (MinigameEffectManager::GetSingleton()->IsRegistered(a_actor))
        {
            MinigameEffectManager::GetSingleton()->GetActorControl(a_actor)->stamina += loc_stamina;
        }
    }
    void MinigameEffectUpdateMagicka(PAPYRUSFUNCHANDLE, RE::Actor *a_actor, float f_magicka)
    {
        //LOG("MinigameEffectUpdateMagicka called")
        float loc_magicka    = -1.0f*UDCONVERTMULT*f_magicka/60.0f;
        if (MinigameEffectManager::GetSingleton()->IsRegistered(a_actor))
        {
            MinigameEffectManager::GetSingleton()->GetActorControl(a_actor)->magicka += loc_magicka;
        }
    }

    void MinigameEffectManager::RegisterActor(RE::Actor *a_actor, float f_mult, float f_stamina, float f_health, float f_magicka, bool b_toggle)
    {
        if (a_actor == nullptr) return;
        //LOG("::RegisterActor - actor={},new size={}",a_actor->GetName(),_actormap.size());
        ActorControl loc_ac = ActorControl{f_stamina,f_health,f_magicka,f_mult,b_toggle};
        _actormap[a_actor] = loc_ac;
        
    }

    void MinigameEffectManager::RemoveActor(RE::Actor *a_actor)
    {
        if (a_actor == nullptr) return;
        _actormap.erase(a_actor);
    }

    bool MinigameEffectManager::IsRegistered(RE::Actor *a_actor)
    {
        if (a_actor == nullptr) return false;
        return (_actormap.find(a_actor) != _actormap.end());
    }

    MinigameEffectManager::ActorControl* MinigameEffectManager::GetActorControl(RE::Actor *a_actor)
    {
        if (a_actor == nullptr) nullptr;
        if (IsRegistered(a_actor))
        {
            return &_actormap[a_actor];
        }
        else
        {
            return nullptr;
        }
    }

    void MinigameEffectManager::RemoveAll(void)
    {
        _actormap.clear();
    }

    inline MinigameEffectManager::ActorControl::ActorControl()
    {
            stamina =  0.0;
            health  =  0.0;
            magicka =  0.0;
            mult    =  1.0;
            toggle  =  true;
    }

    inline MinigameEffectManager::ActorControl::ActorControl(float f_stamina, float f_health, float f_magicka,float f_mult, bool b_toggle)
    {
            stamina =  f_stamina;
            health  =  f_health;
            magicka =  f_magicka;
            mult    =  f_mult;
            toggle  =  b_toggle;
    }
    inline MinigameEffectManager::ActorControl& MinigameEffectManager::ActorControl::operator=(const MinigameEffectManager::ActorControl& source)
    {
        if (this == &source) return *this;
        this->stamina   = source.stamina.load();
        this->health    = source.health.load();
        this->magicka   = source.magicka.load();
        this->mult      = source.mult.load();
        this->toggle    = source.toggle.load();
        return *this;
    }

    void MinigameEffectManager::UpdateMinigameEffect(RE::Actor* a_actor, const float& a_delta)
    {
        if (a_actor == nullptr) return;
        if (IsRegistered(a_actor)){
            float loc_timemult = a_delta/(1.0f/60.0f);  //normalize to 60 fps
            if ( loc_timemult > 0.0 )
            {
                MinigameEffectManager::ActorControl*        loc_ac      = MinigameEffectManager::GetActorControl(a_actor);
                RE::ActorValueOwner* loc_avholder                       = a_actor->AsActorValueOwner();
                if (loc_ac->toggle)
                {
                    _DamageAV(loc_avholder,RE::ActorValue::kHealth,loc_ac->health*loc_ac->mult*loc_timemult,5.0f);
                    _DamageAV(loc_avholder,RE::ActorValue::kStamina,loc_ac->stamina*loc_ac->mult*loc_timemult,0.0);
                    _DamageAV(loc_avholder,RE::ActorValue::kMagicka,loc_ac->magicka*loc_ac->mult*loc_timemult,0.0);
                }
            }
        }
    }

    void MinigameEffectManager::UpdateMeters(const float& a_delta)
    {
        float loc_timemult = a_delta/(1.0f/60.0f);
        if (loc_timemult > 0.0) MeterManager::Update(loc_timemult);
    }
}