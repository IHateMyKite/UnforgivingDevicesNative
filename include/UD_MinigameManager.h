#pragma once
#include <PrismaUI_API.h>
#include <lua.hpp>
#include <UD_Lua.h>
#include <UD_ControlManager.h>

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
    inline const auto MinigameSerData = _byteswap_ulong('UDMG');

    enum class MinigameState
    {
        eNotStarted,
        eStarting,
        eRunning,
        eEnding
    };

    enum class MinigameUIState
    {
        eNotStarted,
        eShown,
        eHidden
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
        int         priority;
        std::string base;
        std::vector<std::string> includes;
    };

    struct MinigameConfigJson;

    typedef std::shared_ptr<MinigameConfigJson> MinigameSetting;

    struct MinigameConfigJson
    {
        uint32_t id;
        std::shared_ptr<boost::property_tree::ptree> json;
        MinigameConfigStatus status;
        std::string error;
        MinigameConfig config;
        MinigameSetting base;
    };

    struct MinigameActionCallback
    {
        Control control;
        std::string callback;
    };

    struct MinigameData
    {
        int id = 0;
        RE::Actor* Wearer   = nullptr;
        RE::Actor* Helper   = nullptr;
        DeviceObj2  Device;
        MinigameSetting Setting;
        MinigameState State = MinigameState::eNotStarted;
        std::vector<MinigameActionCallback> Controls;
        string Context;
    };

    struct MinigamePersData
    {
        int id                              = 0;
        uint32_t Wearer                     = 0;
        uint32_t Helper                     = 0;
        RE::VMHandle    DeviceHandle        = 0;
        char            MinigameName[32U]   = {};
        MinigameState   State               = MinigameState::eNotStarted;
        char            Context[16U]        = {};
        uint16_t        DataSize            = 0;
    };
    static_assert(sizeof(MinigamePersData) == 80U);

    struct MinigameSaveData
    {
        MinigamePersData    Header;
        string              RuntimeData;
    };

    typedef std::shared_ptr<MinigameData> MinigameDataPtr;

    class MinigameManager
    {
        SINGLETONHEADER(MinigameManager)
        public:
            void Reload();
            std::vector<std::string> GetListOfMinigamesStr(RE::Actor* a_actor, RE::TESObjectARMO* a_id);
            std::vector<MinigameSetting> GetListOfMinigames(RE::Actor* a_actor, RE::Actor* a_helper,RE::TESObjectARMO* a_id);

            bool GetMinigameCondition(RE::Actor* a_actor, RE::Actor* a_helper,RE::TESObjectARMO* a_id, MinigameSetting a_setting);
            string GetMinigameContexts(RE::Actor* a_actor, RE::Actor* a_helper, RE::TESObjectARMO* a_id, MinigameSetting a_setting);

            bool StartMinigame(MinigameSetting a_minigame,RE::Actor* a_actor, RE::Actor* a_helper, RE::TESObjectARMO* a_id, string a_cntx);
            bool GetMinigameById(uint32_t a_id,MinigameSetting& a_output);
            MinigameDataPtr GetMinigameDataById(uint32_t a_id);

            bool StopMinigame(RE::Actor* a_actor);

            void Update(float a_delta);

            //void SendCallback(MinigameCallback a_callback);
            void SetMinigameState(MinigameState a_state);
            void StopMinigame(int a_id);
            void OpenMinigameUI(int a_id,std::string a_callback);
            void CloseMinigameUI(int a_id);
            void SetViewReady() {_viewReady = true; _UIState = MinigameUIState::eShown;}
            void InvokeUI(std::string a_command);
            void CheckActionCallback(uint32_t a_dxcode);
            void SendOpenMinigameUICallback();
            void SendPapCallback(int a_id,std::string a_callback,VariableValue& a_var);
            lua_State* GetMinigameScriptById(int a_id);

            void OnGameLoaded(SKSE::SerializationInterface* serde,uint32_t a_type, uint32_t a_size, uint32_t a_version);
            void OnGameSaved(SKSE::SerializationInterface* serde);
            void OnRevert(SKSE::SerializationInterface* serde);
        private:
            MinigameCallback ParseCallback(std::string a_callback);
            bool InitMinigameConfig(MinigameSetting a_config);
            bool OpenMinigameScript(MinigameSetting a_config);
            lua_State* GetMinigameScript(MinigameSetting a_config);
            void UpdateMinigame(MinigameData& a_data,float a_delta);
            void PushMinigameData(lua_State* L,MinigameData& a_data);

            void LoadSavedMinigames();
        private:
            static PRISMA_UI_API::IVPrismaUI1* PrismaUI;
            PrismaView  _view = 0x0UL;
            bool        _viewReady = false;
            MinigameUIState _UIState = MinigameUIState::eNotStarted;

            std::string _callback = "";
            int         _focusedMinigameId = 0;
            std::vector<MinigameSaveData> _MinigameSaves;
            int         _minigameCntr = 0;


            uint8_t     _saveBuffer[65535U];
            uint16_t    _saveBufferReadCount;
            //MinigameData _data;
            bool _init = false;
            std::unordered_map<std::string,MinigameSetting> _jsoncache;
            std::unordered_map<std::string,lua_State*> _scripts;
            std::vector<MinigameDataPtr> _minigames;
            mutable Utils::Spinlock  _lock;
    };

    inline std::vector<std::string> GetListOfMinigames(PAPYRUSFUNCHANDLE, RE::Actor* a_actor, RE::TESObjectARMO* a_id)
    {
        return MinigameManager::GetSingleton()->GetListOfMinigamesStr(a_actor,a_id);
    }

    inline bool StopMinigame(PAPYRUSFUNCHANDLE, RE::Actor* a_actor)
    {
        return MinigameManager::GetSingleton()->StopMinigame(a_actor);
    }

}