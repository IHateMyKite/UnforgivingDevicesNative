#pragma once

namespace UD
{
    class PlayerStatus
    {
    SINGLETONHEADER(PlayerStatus)
    public:
        enum Status : uint8_t
        {
            sBound      = 0x01,
            sMinigame   = 0x02,
            sAnimation  = 0x04
        };
        void Setup();

        void Update();

        Status GetPlayerStatus();
    private:
        bool                            _installed          = false;
        RE::BGSKeyword*                 _hbkeyword          = nullptr;
        RE::TESFaction*                 _minigamefaction    = nullptr;
        RE::TESFaction*                 _animationfaction   = nullptr;
        Status                          _status;

        inline bool PlayerIsBound() const;
        inline bool PlayerInMinigame() const;
        inline bool PlayerInZadAnimation() const;
    };
}