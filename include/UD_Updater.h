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
        void UpdateThread1();
        void UpdateThread2(float a_delta);
    private:
        int _installed = false;
    };
};