#pragma once

#include <UD_Config.h>

#include <PrismaUI_API.h>
#include <MagelightUI_Mod.h>

namespace UD 
{
    using ViewId = std::uint64_t;  // 0 = invalid
    using JsListenerFn = void (*)(const char* argument);
    typedef void (*OnDomReadyCallback)(ViewId view);
    typedef void (*ConsoleCallbackType)(ViewId view, uint8_t level, const char* message);

    enum UIMods
    {
        PrismaUI,
        Magelight,
        None
    };

    enum class UILayer : std::uint8_t 
    { 
        Hud     = 0,
        Panel   = 1,
        Popup   = 2,
        System  = 3
    };

    inline void ConsoleCallback(ViewId view, uint8_t level, const char* message)
    {
        DEBUG("JS[ {} ]",message)
    }

    class UIManager
    {
    SINGLETONHEADER(UIManager)
    public:
        void Reload();

        ViewId CreateView(string a_name, string a_path, OnDomReadyCallback a_callback = nullptr, UILayer a_layer = UILayer::Hud, bool a_clickthrough = true);
        bool DestroyView(ViewId a_id);
        void RegisterConsoleCallback(ViewId a_id,ConsoleCallbackType a_callback);
        void ShowView(ViewId a_id, bool a_show);
        void FocusView(ViewId a_id, bool a_focus);
        void RegisterJSListener(ViewId a_id, string a_name, JsListenerFn a_callback);
        void Invoke(ViewId a_id, string a_js);
        void InteropCall(ViewId a_id, string a_fun, string a_arg);

    public:
        static PRISMA_UI_API::IVPrismaUI2* PrismaUI;
        static MAGELIGHT_API::MagelightMod* Magelight;
    private:
        UIMods GetUsedUIMod();
    private:
        bool _Ready = false;

    };
}