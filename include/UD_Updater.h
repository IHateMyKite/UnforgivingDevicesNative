#pragma once

namespace UD 
{
    //class UpdateManager;

    DWORD WINAPI ThreadUpdate(LPVOID lpParameter);

    class UpdateManager
    {
    public:

        static int CreateUpdateThread(void);
    public:
        static bool BREAK;

        static int PLAYERCONSTRAINS;

    private:
        static HANDLE _threadhandle;
    };
};