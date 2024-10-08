#pragma once

#include <UD_Utility.h>

namespace UD
{
    class PlayerStatus
    {
    SINGLETONHEADER(PlayerStatus)
    public:
        enum Status : uint8_t
        {
            sBound              = 0x01,
            sMinigame           = 0x02,
            sAnimation          = 0x04,
            sHaveTelekinesis    = 0x08
        };
        void Setup();

        void Update();

        Status GetPlayerStatus();

        inline bool PlayerIsBound() const;
        inline bool PlayerInMinigame() const;
        inline bool PlayerInZadAnimation() const;
        inline bool PlayerHaveTelekinesis() const;
    private:
        bool                            _installed          = false;
        RE::BGSKeyword*                 _hbkeyword          = nullptr;
        RE::TESFaction*                 _minigamefaction    = nullptr;
        RE::TESFaction*                 _animationfaction   = nullptr;
        RE::SpellItem*                  _telekinesis        = nullptr;
        Status                          _status;
    };
}