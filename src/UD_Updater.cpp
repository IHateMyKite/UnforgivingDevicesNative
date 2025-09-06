#include <UD_Updater.h>

#include <UD_Config.h>
#include <UD_ControlManager.h>
#include <UD_MinigameEffect.h>
#include <UD_ActorSlotManager.h>
#include <UD_PlayerStatus.h>
#include <UD_ModuleManager.h>
#include <OrgasmSystem/OrgasmManager.h>

SINGLETONBODY(UD::UpdateManager)

namespace UD
{
    void UpdateManager::UpdateThread1(const float& a_delta)
    {
        if (t1mutex) return;
        t1mutex = true;

        SKSE::GetTaskInterface()->AddTask([]
        {
            ControlManager::GetSingleton()->UpdateControl();
        });

        std::this_thread::sleep_for(std::chrono::milliseconds(250)); //only once per 250ms

        t1mutex = false;
    }

    void UpdateManager::Hook()
    {
        HookVirtualMethod<RE::Actor,decltype(PlayerUpdatePatched)>(RE::PlayerCharacter::GetSingleton(),0x0AD,0x0AF,reinterpret_cast<uintptr_t>(PlayerUpdatePatched),PlayerUpdate);

        //auto loc_rel2 = REL::Relocation<uint64_t>(REL::RelocationID(261397,0),REL::VariantOffset(0,0,0)); //vtable of character update
        //HookVirtualMethod<decltype(CharacterUpdatePatched)>(loc_rel2,0x0AD,0x0AF,reinterpret_cast<uintptr_t>(CharacterUpdatePatched),CharacterUpdate);
    }

    void UpdateManager::Update(float a_delta)
    {
        static RE::PlayerCharacter* loc_player = RE::PlayerCharacter::GetSingleton();

        PlayerStatus::GetSingleton()->Update();

        ORS::OrgasmManager::GetSingleton()->Update(a_delta);

        if (!t1mutex) std::thread(&UpdateManager::UpdateThread1,this,a_delta).detach();
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
        if (!t4mutex) std::thread([this]
        {
            if (t4mutex) return;
            t4mutex = true;
            SKSE::GetTaskInterface()->AddTask([]
            {
                ModuleManager::GetSingleton()->Update();
            });
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
            t4mutex = false;
        }).detach();

        MinigameEffectManager::GetSingleton()->UpdateMinigameEffect(loc_player,a_delta);
        MinigameEffectManager::GetSingleton()->UpdateMeters(a_delta);

        //ControlManager::GetSingleton()->UpdateControl();
    }

    void UpdateManager::PlayerUpdatePatched(RE::Actor* a_this, float a_delta)
    {
        static RE::PlayerCharacter* loc_player = RE::PlayerCharacter::GetSingleton();

        if (a_this == loc_player)
        {   
            UpdateManager::GetSingleton()->Update(a_delta);
        }

        UpdateManager::GetSingleton()->PlayerUpdate(a_this,a_delta);
    }

    void UpdateManager::CharacterUpdatePatched(RE::Actor* a_this, float a_delta)
    {
        UpdateManager::GetSingleton()->CharacterUpdate(a_this,a_delta);
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