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
        eDeviceMenu = 1,
        eDeviceMenuSingle = 2
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
        bool ShowDeviceMenuSingle(RE::TESObjectARMO* a_id, RE::TESObjectARMO* a_rd, RE::Actor* a_actor, RE::Actor* a_helper, std::vector<std::string> a_callbacks);
        bool IsMenuOpen();
        void HideMenu(UIMenu a_type);
        void SendCallback(int a_indxDev, int a_indxCall);
        void StartMinigame(int a_indxDev, int a_minId, string a_cntx);
    private:
        std::vector<ButtonCallback> ParseCallbacks(std::vector<std::string> a_callbacks);
        void AddDeviceToList(RE::Actor* a_wearer, RE::Actor* a_helper, RE::TESObjectARMO* a_rd, Object a_obj, std::vector<std::string>& a_list);

        void CreateValueDetail(std::string& a_input, bool a_sep, std::string a_name, std::string a_value, std::string a_id);

        static PRISMA_UI_API::IVPrismaUI1* PrismaUI;
        static bool ViewReady;
        static bool ViewReadySingle;
        PrismaView      _view = 0x0UL;
        PrismaView      _viewSingle = 0x0UL;
        UIMenu          _currentMenu = eNone;
        DeviceMenuData  _devMenuData;
    };

    inline bool ShowDeviceMenu(PAPYRUSFUNCHANDLE, RE::Actor* a_actor, RE::Actor* a_helper, std::vector<std::string> a_callbacks)
    {
        return DeviceMenu::GetSingleton()->ShowDeviceMenu(a_actor,a_helper,a_callbacks);
    }

    inline bool ShowDeviceMenuSingle(PAPYRUSFUNCHANDLE, RE::TESObjectARMO* a_id, RE::TESObjectARMO* a_rd, RE::Actor* a_actor, RE::Actor* a_helper, std::vector<std::string> a_callbacks)
    {
        return DeviceMenu::GetSingleton()->ShowDeviceMenuSingle(a_id,a_rd,a_actor,a_helper,a_callbacks);
    }

    inline bool IsDeviceMenuOpen(PAPYRUSFUNCHANDLE)
    {
        return DeviceMenu::GetSingleton()->IsMenuOpen();
    }

}