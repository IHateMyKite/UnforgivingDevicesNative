#include <UD_PlayerStatus.h>

SINGLETONBODY(UD::PlayerStatus)

void UD::PlayerStatus::Setup()
{
    if (!_installed)
    {
        _installed = true;

        RE::TESDataHandler* loc_datahandler = RE::TESDataHandler::GetSingleton();
        _hbkeyword = static_cast<RE::BGSKeyword*>(loc_datahandler->LookupForm(0x05226C,"Devious Devices - Integration.esm"));
        _minigamefaction = static_cast<RE::TESFaction*>(loc_datahandler->LookupForm(0x150DA3,"UnforgivingDevices.esp"));
        _animationfaction = static_cast<RE::TESFaction*>(loc_datahandler->LookupForm(0x029567,"Devious Devices - Integration.esm"));
    }
}

void UD::PlayerStatus::Update()
{
    uint8_t loc_res = 0x00;
    loc_res |= PlayerIsBound() ? sBound : 0x00;
    loc_res |= PlayerInMinigame() ? sMinigame : 0x00;
    loc_res |= PlayerInZadAnimation() ? sAnimation : 0x00;
    _status = static_cast<Status>(loc_res);
}

UD::PlayerStatus::Status UD::PlayerStatus::GetPlayerStatus()
{
    return _status;
}

bool UD::PlayerStatus::PlayerIsBound() const
{
    RE::Actor* loc_player = RE::PlayerCharacter::GetSingleton();

    if (loc_player == nullptr) return false;

    return Utility::GetSingleton()->WornHasKeyword(loc_player,_hbkeyword);
}

bool UD::PlayerStatus::PlayerInMinigame() const
{
    RE::Actor* loc_player = RE::PlayerCharacter::GetSingleton();

    if (loc_player == nullptr) return false;

    if (loc_player == nullptr || _minigamefaction == nullptr) return false;

    return loc_player->IsInFaction(_minigamefaction);
}

bool UD::PlayerStatus::PlayerInZadAnimation() const
{
    RE::Actor* loc_player = RE::PlayerCharacter::GetSingleton();

    if (loc_player == nullptr) return false;

    if (loc_player == nullptr || _animationfaction == nullptr) return false;

    return loc_player->IsInFaction(_animationfaction);
}