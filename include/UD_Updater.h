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
		
        static void ActorUpdatePatched(RE::Actor* a_this, float a_delta);
        REL::Relocation<decltype(ActorUpdatePatched)> ActorUpdate;

        std::atomic_bool t1mutex = false;
        std::atomic_bool t2mutex = false;
    private:
        int _installed = false;
    };
};