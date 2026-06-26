#pragma once

#include <UD_Config.h>
#include <OrgasmSystem/OrgasmManager.h>
#include <OrgasmSystem/OrgasmData.h>
#include <PrismaUI_API.h>
#include <UD_PapyrusDelegate.h>

namespace UD 
{
    enum UIMenu
    {
        eNone = 0,
        eDeviceMenu = 1
    };

    struct ButtonCallback
    {
        std::string Name;
        std::string Module;
        std::string Callback;
        std::string Argument;
    };

    struct DeviceMenuData
    {
        RE::Actor* Wearer;
        RE::Actor* Helper;
        std::vector<DeviceObj2> List;
        std::vector<ButtonCallback> Callbacks;
    };

    class DeviceMenu
    {
    SINGLETONHEADER(DeviceMenu)
    public:
        void Reload();
        void Update();
        bool ShowDeviceMenu(RE::Actor* a_actor, RE::Actor* a_helper, std::vector<std::string> a_callbacks);
        bool IsMenuOpen();
        void HideMenu(UIMenu a_type);
        void SendCallback(int a_indxDev, int a_indxCall);
        void StartMinigame(int a_indxDev, int a_minId, string a_cntx);
    private:
        std::vector<ButtonCallback> ParseCallbacks(std::vector<std::string> a_callbacks);

        void CreateValueDetail(std::string& a_input, bool a_sep, std::string a_name, std::string a_value, std::string a_id);

        static PRISMA_UI_API::IVPrismaUI1* PrismaUI;
        static bool ViewReady;
        PrismaView      _view = 0x0UL;
        UIMenu          _currentMenu = eNone;
        DeviceMenuData  _devMenuData;
    };

    inline bool ShowDeviceMenu(PAPYRUSFUNCHANDLE, RE::Actor* a_actor, RE::Actor* a_helper, std::vector<std::string> a_callbacks)
    {
        return DeviceMenu::GetSingleton()->ShowDeviceMenu(a_actor,a_helper,a_callbacks);
    }

    inline bool IsMenuOpen(PAPYRUSFUNCHANDLE)
    {
        return DeviceMenu::GetSingleton()->IsMenuOpen();
    }
}