#pragma once

namespace UD
{
    void OnGameLoaded(SKSE::SerializationInterface* serde);
    void OnGameSaved(SKSE::SerializationInterface* serde);
    void OnRevert(SKSE::SerializationInterface*);
}