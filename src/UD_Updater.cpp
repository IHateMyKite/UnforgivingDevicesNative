#include <UD_Updater.h>

namespace UD
{
    bool UpdateManager::BREAK = false;
    int UpdateManager::PLAYERCONSTRAINS = 0x00000000;
    HANDLE UpdateManager::_threadhandle = NULL;

    DWORD WINAPI ThreadUpdate(LPVOID lpParameter)
    {
        while(!UpdateManager::BREAK)
        {
            UpdateManager::PLAYERCONSTRAINS = GetActorConstrainsInter(UD::PLAYER);
            UDSKSELOG("ThreadUpdate tick - PLAYERCONSTRAINS = {}",UpdateManager::PLAYERCONSTRAINS)
            Sleep(1000U); //wait one second before next update
        }
        return 0;
    }

    int UpdateManager::CreateUpdateThread(void)
    {
        if (_threadhandle == NULL)
        {
            //https://riptutorial.com/winapi/example/13881/create-a-new-thread
            _threadhandle = CreateThread(
                NULL,    // Thread attributes
                0,       // Stack size (0 = use default)
                ThreadUpdate, // Thread start address
                NULL,    // Parameter to pass to the thread
                0,       // Creation flags
                NULL);   // Thread id
            if (_threadhandle == NULL)
            {
                UDSKSELOG("Update Thread creation failed")
                // Thread creation failed.
                // More details can be retrieved by calling GetLastError()
                return 1;
            }
            UDSKSELOG("Update Thread created succefully")
            return  0;
        }
        else
        {
            UDSKSELOG("Update Thread already exist")
            return 2;
        }
        
    }
}