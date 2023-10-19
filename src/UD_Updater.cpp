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
        if (!RE::UI::GetSingleton()->GameIsPaused())
        {
            ActorSlotManager::GetSingleton()->Update();
        }
        else
        {
            //do nothing
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
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
            std::thread loc_thrd1(&UpdateManager::UpdateThread1,UpdateManager::GetSingleton(),a_delta);
            loc_thrd1.detach();

            std::thread loc_thrd2(&UpdateManager::UpdateThread2,UpdateManager::GetSingleton(),a_delta);
            loc_thrd2.detach();

            MinigameEffectManager::GetSingleton()->UpdateMinigameEffect(a_this,a_delta);
            MinigameEffectManager::GetSingleton()->UpdateMeters(a_this,a_delta);

            //UpdateManager::GetSingleton()->CallSerTasks();
        }
        UpdateManager::GetSingleton()->ActorUpdate(a_this,a_delta);
    }

    bool UpdateManager::AddSerTask(std::function<void(void*)> a_task, void* a_arg, bool a_freearg)
    {
        std::unique_lock lock(_taskmutex);
        _taskstack.push_back({a_arg,a_task,a_freearg});
        return true;
    }

    void UpdateManager::CallSerTasks()
    {
        std::unique_lock lock(_taskmutex);

        for (auto&& it : _taskstack)
        {
            it.task(it.arg);
            if (it.freearg) delete it.arg;
        }
        _taskstack.clear();
    }

    void UpdateManager::CreateUpdateThreads(void)
    {
        if (!_installed)
        {
            Hook();
            //std::thread loc_thrd1(&UpdateManager::UpdateThread1,this);
            //loc_thrd1.detach();

            //std::thread loc_thrd2(&UpdateManager::UpdateThread2,this);
            //loc_thrd2.detach();
            _installed = true;
        }

    }
}