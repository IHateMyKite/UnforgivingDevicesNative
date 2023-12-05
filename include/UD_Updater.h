#pragma once

namespace UD 
{
    class UpdateManager
    {
    SINGLETONHEADER(UpdateManager)
    public:
        void CreateUpdateThreads(void);
    public:
        void UpdateThread1(const float& a_delta);   //used for slow update
        void UpdateThread2(const float& a_delta);   //used for fps tied updated (is called on every frame). Should be fast

        void Hook();
        
        void Update(float a_delta);

        static void ActorUpdatePatched(RE::Actor* a_this, float a_delta);
        REL::Relocation<decltype(ActorUpdatePatched)> ActorUpdate;

        bool t1mutex = false;
        bool t2mutex = false;
        bool t3mutex = false;
    private:
        int _installed = false;
    };
};