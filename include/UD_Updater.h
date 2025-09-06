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

        void Hook();
        
        void Update(float a_delta);

        static void PlayerUpdatePatched(RE::Actor* a_this, float a_delta);
        REL::Relocation<decltype(PlayerUpdatePatched)> PlayerUpdate;

        static void CharacterUpdatePatched(RE::Actor* a_this, float a_delta);
        REL::Relocation<decltype(CharacterUpdatePatched)> CharacterUpdate;

        bool t1mutex = false;
        bool t3mutex = false;
        bool t4mutex = false;
    private:
        int _installed = false;
    };
};