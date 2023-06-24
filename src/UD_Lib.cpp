#include <UD_Lib.h>

namespace UD
{
    void ReloadLib()
    {
        PLAYER = RE::PlayerCharacter::GetSingleton();
    }
}