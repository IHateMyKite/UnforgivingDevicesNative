#include <UD_MinigameEffect.h>

namespace UD
{
    std::atomic<bool> ActorValueUpdateHook::started                                         = std::atomic<bool>(false);
    std::map<RE::Actor*,ActorValueUpdateHook::ActorControl> ActorValueUpdateHook::_actormap = std::map<RE::Actor*,ActorValueUpdateHook::ActorControl>();
    //RE::PlayerCharacter* ActorValueUpdateHook::player = RE::PlayerCharacter::GetSingleton();

    inline bool _DamageAV(RE::ActorValueOwner* a_avowner,const RE::ActorValue& a_av, const float& f_dmg, const float& f_min)
    {
        if ((f_dmg != 0.0) && ((a_avowner->GetActorValue(a_av) - f_dmg) > f_min))
        {
            a_avowner->RestoreActorValue(RE::ACTOR_VALUE_MODIFIER::kDamage,a_av,f_dmg);
        }
        return true;
    }

    // Function ProcessAVMinigame(Actor akActor, Float afStamina, Float afHealth, Float afMagicka) global native
    void StartMinigameEffect(PAPYRUSFUNCHANDLE, RE::Actor *a_actor, float f_mult, float f_stamina, float f_health, float f_magicka, bool b_toggle) 
    {
        //The value in papyrus was not corresponding to real value because of lag. 
        //For that reason, we need to reduce the drain value, otherwise it will be too fast
        //For now, value is reduced by 40%
        float loc_health    = -1.0f*UDCONVERTMULT*f_health/60.0f;
        float loc_stamina   = -1.0f*UDCONVERTMULT*f_stamina/60.0f;
        float loc_magicka   = -1.0f*UDCONVERTMULT*f_magicka/60.0f;
        float loc_mult      = f_mult; 
        float loc_toggle    = b_toggle;

        ActorValueUpdateHook::RegisterActor(a_actor,loc_mult,loc_stamina,loc_health,loc_magicka,loc_toggle);

        SKSE::log::info("::StartMinigameEffect - health={},stamina={},magicka={}",loc_health,loc_stamina,loc_magicka);
    }

    void EndMinigameEffect(PAPYRUSFUNCHANDLE, RE::Actor *a_actor) 
    {
        SKSE::log::info("::EndMinigameEffect called");
        ActorValueUpdateHook::RemoveActor(a_actor);
    }

    bool IsMinigameEffectOn(PAPYRUSFUNCHANDLE, RE::Actor *a_actor) 
    {
        return ActorValueUpdateHook::started;
    }

    void UpdateMinigameEffectMult(PAPYRUSFUNCHANDLE, RE::Actor *a_actor, float f_mult) 
    {
        if (ActorValueUpdateHook::IsRegistered(a_actor))
        {
            ActorValueUpdateHook::GetActorControl(a_actor)->mult = f_mult;
        }
    }

    void ToggleMinigameEffect(PAPYRUSFUNCHANDLE, RE::Actor *a_actor, bool b_toggle) 
    {
        SKSE::log::info("::ToggleMinigameEffect called {}",b_toggle);
        if (ActorValueUpdateHook::IsRegistered(a_actor))
        {
            ActorValueUpdateHook::GetActorControl(a_actor)->toggle = b_toggle;
        }
    }

    bool MinigameStatsCheck(PAPYRUSFUNCHANDLE, RE::Actor *a_actor) 
    {
        RE::ActorValueOwner* a_avowner   = a_actor->AsActorValueOwner();
        bool loc_res = true;
        loc_res &= a_avowner->GetActorValue(RE::ActorValue::kStamina) > 0.1;
        loc_res &= a_avowner->GetActorValue(RE::ActorValue::kHealth)  > 5.1;
        loc_res &= a_avowner->GetActorValue(RE::ActorValue::kMagicka) > 0.1;
        return loc_res;
    }

    void MinigameEffectUpdateHealth(PAPYRUSFUNCHANDLE, RE::Actor *a_actor, float f_health)
    {
        float loc_health    = -1.0f*UDCONVERTMULT*f_health/60.0f;
        if (ActorValueUpdateHook::IsRegistered(a_actor))
        {
            ActorValueUpdateHook::GetActorControl(a_actor)->health = loc_health;
        }
    }
    void MinigameEffectUpdateStamina(PAPYRUSFUNCHANDLE, RE::Actor *a_actor, float f_stamina)
    {
        float loc_stamina    = -1.0f*UDCONVERTMULT*f_stamina/60.0f;
        if (ActorValueUpdateHook::IsRegistered(a_actor))
        {
            ActorValueUpdateHook::GetActorControl(a_actor)->stamina = loc_stamina;
        }
    }
    void MinigameEffectUpdateMagicka(PAPYRUSFUNCHANDLE, RE::Actor *a_actor, float f_magicka)
    {
        float loc_magicka    = -1.0f*UDCONVERTMULT*f_magicka/60.0f;
        if (ActorValueUpdateHook::IsRegistered(a_actor))
        {
            ActorValueUpdateHook::GetActorControl(a_actor)->magicka = loc_magicka;
        }
    }

	void ActorValueUpdateHook::Patch()
	{
        SKSE::log::info("ActorValueUpdateHook::Patch()");
        if (!started)
        {
			auto& trampoline = SKSE::GetTrampoline();
            trampoline.create(14);
 
			REL::Relocation<uintptr_t> ActorValueUpdateFun{ REL::RelocationID { 37509, 38451 }, 0x19 };

			ActorValueUpdate = trampoline.write_call<5>(ActorValueUpdateFun.address(), ActorValueUpdatePatched);
            started = true;
        }
	}


	int32_t ActorValueUpdateHook::ActorValueUpdatePatched( RE::Character* a_actor, RE::ActorValue a_av, float a_unk)
	{
        //SKSE::log::info("::ActorValueUpdatePatched - a_actor={}",a_actor->GetName());
        //RE::ConsoleLog::GetSingleton()->Print(std::format("::ActorValueUpdate health={},stamina={},magicka={},mult={},toggle={}",health,stamina,magicka,mult,toggle).c_str());
        if ((a_actor != nullptr) && (IsRegistered(a_actor)))
        {
            float loc_timemult = RE::GetSecondsSinceLastFrame()/(1.0f/60.0f);  //normalize to 60 fps
            if ( loc_timemult > 0.0 ) 
            {
                ActorControl*        loc_ac       = GetActorControl(a_actor);
                RE::ActorValueOwner* loc_avholder = a_actor->AsActorValueOwner();
                if (loc_ac->toggle)
                {
                    _DamageAV(loc_avholder,RE::ActorValue::kHealth,loc_ac->health*loc_ac->mult*loc_timemult,5.0f);
                    _DamageAV(loc_avholder,RE::ActorValue::kStamina,loc_ac->stamina*loc_ac->mult*loc_timemult,0.0);
                    _DamageAV(loc_avholder,RE::ActorValue::kMagicka,loc_ac->magicka*loc_ac->mult*loc_timemult,0.0);
                }
            }
        }

        //check if actor is player
        static RE::PlayerCharacter* loc_player = RE::PlayerCharacter::GetSingleton();
        if (loc_player == a_actor)
        {   
            float loc_timemult = RE::GetSecondsSinceLastFrame()/(1.0f/60.0f);
            if (loc_timemult > 0.0) MeterManager::Update(loc_timemult);
        }

		return ActorValueUpdate(a_actor, a_av, a_unk);
	}

    inline void ActorValueUpdateHook::RegisterActor(RE::Actor *a_actor, float f_mult, float f_stamina, float f_health, float f_magicka, bool b_toggle)
    {
        ActorControl loc_ac = ActorControl{f_stamina,f_health,f_magicka,f_mult,b_toggle};
        _actormap[a_actor] = loc_ac;
        SKSE::log::info("::RegisterActor - actor={},new size={}",a_actor->GetName(),_actormap.size());
        //ActorValueUpdateHook::Patch();
    }

    inline void ActorValueUpdateHook::RemoveActor(RE::Actor *a_actor)
    {
        _actormap.erase(a_actor);
    }

    inline bool ActorValueUpdateHook::IsRegistered(RE::Actor *a_actor)
    {
          return (_actormap.find(a_actor) != _actormap.end());
    }

    inline ActorValueUpdateHook::ActorControl* ActorValueUpdateHook::GetActorControl(RE::Actor *a_actor)
    {
        if (IsRegistered(a_actor))
        {
            return &_actormap[a_actor];
        }
        else
        {
            return nullptr;
        }
    }

    void ActorValueUpdateHook::RemoveAll(void)
    {
        _actormap.clear();
    }

    inline ActorValueUpdateHook::ActorControl::ActorControl()
    {
            stamina =  0.0;
            health  =  0.0;
            magicka =  0.0;
            mult    =  1.0;
            toggle  =  true;
    }

    inline ActorValueUpdateHook::ActorControl::ActorControl(float f_stamina, float f_health, float f_magicka,float f_mult, bool b_toggle)
    {
            stamina =  f_stamina;
            health  =  f_health;
            magicka =  f_magicka;
            mult    =  f_mult;
            toggle  =  b_toggle;
    }
    inline ActorValueUpdateHook::ActorControl& ActorValueUpdateHook::ActorControl::operator=(const ActorValueUpdateHook::ActorControl& source)
    {
        if (this == &source) return *this;
        this->stamina   = source.stamina.load();
        this->health    = source.health.load();
        this->magicka   = source.magicka.load();
        this->mult      = source.mult.load();
        this->toggle    = source.toggle.load();
        return *this;
    }
}