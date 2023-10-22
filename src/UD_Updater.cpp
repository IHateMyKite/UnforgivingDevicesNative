#include <UD_Updater.h>

#include <Windows.h>

SINGLETONBODY(UD::UpdateManager)

namespace UD
{
    void UpdateManager::UpdateThread1(const float& a_delta)
    {
        static std::atomic_bool loc_mutex = false;
        if (loc_mutex) return;
        loc_mutex = true;

        ActorSlotManager::GetSingleton()->Update();

        std::this_thread::sleep_for(std::chrono::milliseconds(1000)); //only once per 1s

        loc_mutex = false;
    }

    void UpdateManager::UpdateThread2(const float& a_delta)
    {
        static std::atomic_bool loc_mutex = false;
        if (loc_mutex) return;
        loc_mutex = true;

        ORS::OrgasmManager::GetSingleton()->Update(a_delta);

        loc_mutex = false;
    }

    void UpdateManager::Hook()
    {
        HookVirtualMethod<RE::Actor,decltype(ActorUpdatePatched)>(RE::PlayerCharacter::GetSingleton(),0x0AD,0x0AF,reinterpret_cast<uintptr_t>(ActorUpdatePatched),ActorUpdate);
    }

    void UpdateManager::ActorUpdatePatched(RE::Actor* a_this, float a_delta)
    {
        static RE::PlayerCharacter* loc_player = RE::PlayerCharacter::GetSingleton();

        if (a_this == loc_player)
        {   
            std::thread(&UpdateManager::UpdateThread1,UpdateManager::GetSingleton(),a_delta).detach();
            std::thread(&UpdateManager::UpdateThread2,UpdateManager::GetSingleton(),a_delta).detach();

            MinigameEffectManager::GetSingleton()->UpdateMinigameEffect(a_this,a_delta);
            MinigameEffectManager::GetSingleton()->UpdateMeters(a_this,a_delta);
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