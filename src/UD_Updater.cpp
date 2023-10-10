#include <UD_Updater.h>

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

    void UpdateManager::UpdateThread2()
    {
        //ORS::OrgasmManager::GetSingleton()->AddOrgasmChange(PLAYER,"TestKey",ORS::mNone,ORS::eVagina1 | ORS::eClitoris,1.0f,0,0,0,0,0);
        while (true)
        {
            if (!RE::UI::GetSingleton()->GameIsPaused())
            {
                ORS::OrgasmManager::GetSingleton()->Update(ORSUPTIME/1000.0f);
                //UDSKSELOG("UpdateThread2 tick - Org. prg. = {} , org. rate = {}",ORS::OrgasmManager::GetSingleton()->GetOrgasmProgress(PLAYER),ORS::OrgasmManager::GetSingleton()->GetOrgasmChangeVar(PLAYER,"TestKey",ORS::vOrgasmRate))
                std::this_thread::sleep_for(std::chrono::milliseconds(ORSUPTIME));
            }
            else
            {
                //UDSKSELOG("UpdateThread2 tick - Game is paused")
                std::this_thread::sleep_for(std::chrono::milliseconds(333));
            }
        }
    }

    void UpdateManager::CreateUpdateThreads(void)
    {
        if (!_installed)
        {
            std::thread loc_thrd1(&UpdateManager::UpdateThread1,this);
            loc_thrd1.detach();

            std::thread loc_thrd2(&UpdateManager::UpdateThread2,this);
            loc_thrd2.detach();
            _installed = true;
        }

    }
}