#pragma once
#include <PrismaUI_API.h>
#include <lua.hpp>
#include <UD_Lua.h>

#define LUA_CHECK(x,y) \
{                      \
    if (x != LUA_OK)   \
    {                  \
        ERROR(y)       \
        return;        \
    }                  \
}

#define LUA_CHECKNIL(x,y) \
{                      \
    if (x == LUA_TNIL) \
    {                  \
        ERROR(y)       \
        return;        \
    }                  \
}

namespace UD
{
    enum class MinigameState
    {
        eNotStarted,
        eStarting,
        eRunning,
        eEnding
    };

    struct MinigameData
    {
        RE::Actor* Wearer   = nullptr;
        RE::Actor* Helper   = nullptr;
        Object DeviceObj    = nullptr;
        MinigameState State = MinigameState::eNotStarted;
    };

    struct MinigameCallback
    {
        std::string Module;
        std::string Callback;
        std::string Argument;
    };

    enum class MinigameConfigStatus : uint8_t
    {
        sOK             = 0U,
        sDisabled       = 1U,
        sMissingMaster  = 2U,
        sError          = 3U
    };

    struct MinigameConfig
    {
        std::string name;
        std::string description;
        std::string uiobject;
        std::string script;
        /* TODO: Minigames */
    };

    struct MinigameConfigJson
    {
        std::shared_ptr<boost::property_tree::ptree> json;
        MinigameConfigStatus status;
        std::string error;
        MinigameConfig config;
    };

    class MinigameManager
    {
        SINGLETONHEADER(MinigameManager)
        public:
            void Reload();
            std::vector<std::string> GetListOfMinigames(RE::Actor* a_actor, RE::TESObjectARMO* a_id);
            bool StartMinigame(RE::BGSBaseAlias* a_minigame,RE::Actor* a_actor, RE::TESObjectARMO* a_id);
            //void SendCallback(MinigameCallback a_callback);
            void SetMinigameState(MinigameState a_state);
        private:
            MinigameCallback ParseCallback(std::string a_callback);
            void InitConfig(std::shared_ptr<MinigameConfigJson> a_config);
            void CreateContext(RE::Actor* a_actor, RE::Actor* a_helper, RE::TESObjectARMO* a_id);

        private:
            static PRISMA_UI_API::IVPrismaUI1* PrismaUI;
            PrismaView _view = 0x0UL;
            MinigameData _data;
            bool _init = false;
            std::unordered_map<std::string,std::shared_ptr<MinigameConfigJson>> _jsoncache;
            std::unordered_map<std::string,lua_State*> _scripts;
            std::vector<MinigameData> _minigames;
    };

    inline std::vector<std::string> GetListOfMinigames(PAPYRUSFUNCHANDLE, RE::Actor* a_actor, RE::TESObjectARMO* a_id)
    {
        return MinigameManager::GetSingleton()->GetListOfMinigames(a_actor,a_id);
    }
}