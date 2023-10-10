#pragma once

namespace UD 
{
    class UpdateManager
    {
    SINGLETONHEADER(UpdateManager)
    public:
        void CreateUpdateThreads(void);
    public:
        //bool BREAK;
        std::atomic<int> PLAYERCONSTRAINS;
        
    private:
        int _installed = false;
        void UpdateThread1();
        void UpdateThread2();
    };
};