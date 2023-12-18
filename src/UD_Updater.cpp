#include <UD_Updater.h>

#include <Windows.h>

SINGLETONBODY(UD::UpdateManager)

namespace UD
{
    void UpdateManager::UpdateThread1(const float& a_delta)
    {
        if (t1mutex) return;
        t1mutex = true;

        ControlManager::GetSingleton()->UpdateControl();
        
        std::this_thread::sleep_for(std::chrono::milliseconds(500)); //only once per 500ms

        t1mutex = false;
    }

    void UpdateManager::UpdateThread2(const float& a_delta)
    {
        if (t2mutex) return;
        t2mutex = true;

        ORS::OrgasmManager::GetSingleton()->Update(a_delta);

        t2mutex = false;
    }

    void UpdateManager::Hook()
    {
        HookVirtualMethod<RE::Actor,decltype(ActorUpdatePatched)>(RE::PlayerCharacter::GetSingleton(),0x0AD,0x0AF,reinterpret_cast<uintptr_t>(ActorUpdatePatched),ActorUpdate);
    }

    void UpdateManager::Update(float a_delta)
    {
        static RE::PlayerCharacter* loc_player = RE::PlayerCharacter::GetSingleton();

        PlayerStatus::GetSingleton()->Update();

        if (!t1mutex) std::thread(&UpdateManager::UpdateThread1,this,a_delta).detach();
        if (!t2mutex) std::thread(&UpdateManager::UpdateThread2,this,a_delta).detach();
        if (!t3mutex) std::thread([this]
        {
            if (t3mutex) return;
            t3mutex = true;
            SKSE::GetTaskInterface()->AddTask([]
            {
                ActorSlotManager::GetSingleton()->Update();
            });
            std::this_thread::sleep_for(std::chrono::milliseconds(UD::Config::GetSingleton()->GetVariable<int>("General.iUpdateTime",2500))); //only once per 2.5s
            t3mutex = false;
        }).detach();

        MinigameEffectManager::GetSingleton()->UpdateMinigameEffect(loc_player,a_delta);
        MinigameEffectManager::GetSingleton()->UpdateMeters(a_delta);
            
        //ControlManager::GetSingleton()->UpdateControl();
    }

    void UpdateManager::ActorUpdatePatched(RE::Actor* a_this, float a_delta)
    {
        static RE::PlayerCharacter* loc_player = RE::PlayerCharacter::GetSingleton();

        if (a_this == loc_player)
        {   
            UpdateManager::GetSingleton()->Update(a_delta);
        }

        UpdateManager::GetSingleton()->ActorUpdate(a_this,a_delta);
    }

    void UpdateManager::CreateUpdateThreads(void)
    {
        if (!_installed)
        {
            Hook();
            _installed = true;
        }
    }
}