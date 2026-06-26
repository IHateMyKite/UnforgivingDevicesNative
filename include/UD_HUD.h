#pragma once

#include <UD_Config.h>
#include <UD_Lua.h>
#include <UD_UI.h>
#include <PrismaUI_API.h>

namespace UD 
{
    enum class HudElementState
    {
        eNotStarted,
        eShown,
        eHidden
    };

    enum class HudState
    {
        eNotStarted,
        eShown,
        eHidden
    };

    enum class HudElementConfigStatus : uint8_t
    {
        sOK             = 0U,
        sDisabled       = 1U,
        sMissingMaster  = 2U,
        sError          = 3U
    };

    struct HudElementConfig
    {
        std::string name;
        std::string description;
        std::string script;
        int         priority;
        std::string base;
        bool        abstract;
        std::vector<std::string> includes;
    };

    struct HudElementConfigJson;
    typedef std::shared_ptr<HudElementConfigJson> HudElementSetting;
    struct HudElementConfigJson
    {
        uint32_t id;
        std::shared_ptr<boost::property_tree::ptree> json;
        HudElementConfigStatus status;
        std::string error;
        HudElementConfig config;
        HudElementSetting base;
    };

    // Runtime data
    struct HudElementData
    {
        int id = 0;
        RE::Actor* Target   = nullptr;
        HudElementSetting Setting;
        HudElementState State = HudElementState::eNotStarted;
    };

    typedef std::shared_ptr<HudElementData> HudElementDataPtr;

    class HudManager
    {
    SINGLETONHEADER(HudManager)
    public:
        void Reload();
        void Update(float a_delta);
        void InvokeHud(std::string a_message);
        void SetViewReady() {_viewReady = true; _HudState = HudState::eShown;}
    private:
        bool InitConfig(HudElementSetting a_config);
        bool OpenScript(HudElementSetting a_config);
        void PushHudData(lua_State* L,HudElementData& a_data);
        lua_State* GetElementScript(HudElementSetting a_setting);
    public:
        static PRISMA_UI_API::IVPrismaUI1* PrismaUI;
    private:
        PrismaView  _view = 0x0UL;
        HudState    _HudState = HudState::eNotStarted;
        bool        _viewReady = false;
        bool        _init = false;
        int         _hudcntr = 0;
        std::unordered_map<std::string,HudElementSetting> _jsoncache;
        std::unordered_map<std::string,lua_State*> _scripts;
        std::vector<HudElementDataPtr> _elements;
    };
}