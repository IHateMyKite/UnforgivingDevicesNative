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
        ActorSlotManager::GetSingleton()->Update();

        std::this_thread::sleep_for(std::chrono::milliseconds(500)); //only once per 1s

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

    void UpdateManager::ActorUpdatePatched(RE::Actor* a_this, float a_delta)
    {
        static RE::PlayerCharacter* loc_player = RE::PlayerCharacter::GetSingleton();

        static UpdateManager* loc_manager = UpdateManager::GetSingleton();

        if (a_this == loc_player)
        {   
            if (!loc_manager->t1mutex) std::thread(&UpdateManager::UpdateThread1,loc_manager,a_delta).detach();
            if (!loc_manager->t2mutex) std::thread(&UpdateManager::UpdateThread2,loc_manager,a_delta).detach();

            MinigameEffectManager::GetSingleton()->UpdateMinigameEffect(a_this,a_delta);
            MinigameEffectManager::GetSingleton()->UpdateMeters(a_this,a_delta);
        }
        loc_manager->ActorUpdate(a_this,a_delta);
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