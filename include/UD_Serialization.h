#pragma once

namespace UD
{
    enum LoadMode : int
    {
        mDefault = 0,
        mSafe    = 1,

        TotalNumber
    };

    void OnGameLoaded(SKSE::SerializationInterface* serde);
    void OnGameSaved(SKSE::SerializationInterface* serde);
    void OnRevert(SKSE::SerializationInterface*);
}