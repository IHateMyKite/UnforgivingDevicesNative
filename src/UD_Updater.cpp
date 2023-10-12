#include <UD_Updater.h>

#include <Windows.h>

SINGLETONBODY(UD::UpdateManager)

namespace UD
{
    void UpdateManager::UpdateThread1()
    {
        while (true)
        {
            if (!RE::UI::GetSingleton()->GameIsPaused())
            {
                ActorSlotManager::GetSingleton()->Update();
                //UDSKSELOG("UpdateThread1 tick - Constrains = {}",ActorSlotManager::GetSingleton()->GetActorStorage(RE::PlayerCharacter::GetSingleton()).Constrains)
            }
            else
            {
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(1000));
        }
    }

    void UpdateManager::UpdateThread2(float a_delta)
    {
        //ORS::OrgasmManager::GetSingleton()->AddOrgasmChange(PLAYER,"TestKey",ORS::mNone,ORS::eVagina1 | ORS::eClitoris,1.0f,0,0,0,0,0);
        //while (true)
        {
            //if (!RE::UI::GetSingleton()->GameIsPaused() || RE::BSGraphics::Renderer::GetSingleton()->data.context)
            {
                //UDSKSELOG("Updating orgasm manager")
                ORS::OrgasmManager::GetSingleton()->Update(a_delta/*ORSUPTIME/1000.0f*/);
                //UDSKSELOG("UpdateThread2 tick - Org. prg. = {} , org. rate = {}",ORS::OrgasmManager::GetSingleton()->GetOrgasmProgress(PLAYER),ORS::OrgasmManager::GetSingleton()->GetOrgasmChangeVar(PLAYER,"TestKey",ORS::vOrgasmRate))
                //std::this_thread::sleep_for(std::chrono::milliseconds(ORSUPTIME));
            }
            //else
            {
                //UDSKSELOG("UpdateThread2 tick - Game is paused")
                //std::this_thread::sleep_for(std::chrono::milliseconds(333));
            }
        }
    }

    void UpdateManager::CreateUpdateThreads(void)
    {
        //auto loc_t = GetCurrentThread();
        //auto loc_sc = SuspendThread(loc_t);
        //ResumeThread(loc_t);
        //if (loc_sc > 1)
        //{
        //
        //}
        //else
        //{
        //
        //}
        //auto loc_t = std::this_thread::get_id();
        //std::thread::thread()
        //loc_t.getState();
        if (!_installed)
        {
            std::thread loc_thrd1(&UpdateManager::UpdateThread1,this);
            loc_thrd1.detach();

            //std::thread loc_thrd2(&UpdateManager::UpdateThread2,this);
            //loc_thrd2.detach();
            _installed = true;
        }

    }
}