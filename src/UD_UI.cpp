#include <UD_UI.h>
#include <UD_Utility.h>

SINGLETONBODY(UD::UIManager)

namespace UD
{
    PRISMA_UI_API::IVPrismaUI2* UIManager::PrismaUI = nullptr;
    MAGELIGHT_API::MagelightMod* UIManager::Magelight = nullptr;

    void UIManager::Reload()
    {
        if (!_Ready)
        {
            auto loc_apiptr = PRISMA_UI_API::RequestPluginAPI();
            PrismaUI = reinterpret_cast<PRISMA_UI_API::IVPrismaUI2*>(loc_apiptr);
            if (PrismaUI) DEBUG("PrismaUI found")
            else DEBUG("No PrismaUI found")

            auto loc_magelight = new MAGELIGHT_API::MagelightMod();
            if (loc_magelight->acquire("UnforgivingDevices","UnforgivingDevices",1,8,0))
            {
                DEBUG("Magelight found")
                Magelight = loc_magelight;
            }
            else
            {
                DEBUG("No Magelight found")
                delete loc_magelight;
                Magelight = nullptr;
            }

            DEBUG("Prefered UI mod = {}",Config::GetSingleton()->GetVariable<string>("Ui.sPreferedMod","PrismaUI"))
        }
    }

    ViewId UIManager::CreateView(string a_name, string a_path, OnDomReadyCallback a_callback, UILayer a_layer, bool a_clickthrough)
    {
        ViewId loc_res = 0;

        switch(GetUsedUIMod())
        {
            case UIMods::PrismaUI:
            {
                const string loc_path = "UnforgivingDevices/" + a_path;
                loc_res = PrismaUI->CreateView(loc_path.c_str(),a_callback);
            }
            break;
            case UIMods::Magelight:
            {
                const string loc_path = RelToAbsPath("Magelight/UnforgivingDevices/" + a_path);
                //const string loc_path = a_path;
                //boost::replace_all(loc_path,"/","\\");
                MAGELIGHT_API::MagelightMod::ViewOpts loc_opts;
                loc_opts.fullscreen = true;
                loc_opts.onDomReady = a_callback;
                loc_opts.clickThrough = a_clickthrough;
                loc_res = Magelight->createView(a_name.c_str(), loc_path.c_str(),(MAGELIGHT_API::Layer)a_layer,loc_opts);
                DEBUG("Opening view {} using Magelight -> {} / {}",loc_path,loc_res,Magelight->v4()->GetLastErrorMessage(Magelight->id()));
                DEBUG("Is GPU used = {}",Magelight->v4()->IsGpuAccelerated())
            }
            break;
            default:
            break;
        }

        return loc_res;
    }

    bool UIManager::DestroyView(ViewId a_id)
    {
        bool loc_res = true;
        switch(GetUsedUIMod())
        {
            case UIMods::PrismaUI:
            {
                PrismaUI->Destroy(a_id);
            }
            break;
            case UIMods::Magelight:
            {
                Magelight->v4()->DestroyView(a_id);
            }
            break;
            default:
            break;
        }

        return loc_res;
    }

    void UIManager::RegisterConsoleCallback(ViewId a_id, ConsoleCallbackType a_callback)
    {
        switch(GetUsedUIMod())
        {
            case UIMods::PrismaUI:
            {
                PrismaUI->RegisterConsoleCallback(a_id,reinterpret_cast<PRISMA_UI_API::ConsoleMessageCallback>(a_callback));
            }
            break;
            default:
            break;
        }
    }

    void UIManager::ShowView(ViewId a_id, bool a_show)
    {
        switch(GetUsedUIMod())
        {
            case UIMods::PrismaUI:
            {
                if (a_show) PrismaUI->Show(a_id);
                else PrismaUI->Hide(a_id);
            }
            break;
            case UIMods::Magelight:
            {
                Magelight->v4()->ShowView(a_id,a_show);
            }
            break;
            default:
            break;
        }
    }

    void UIManager::FocusView(ViewId a_id, bool a_focus)
    {
        switch(GetUsedUIMod())
        {
            case UIMods::PrismaUI:
            {
                if (a_focus) PrismaUI->Focus(a_id);
                else PrismaUI->Unfocus(a_id);
            }
            break;
            case UIMods::Magelight:
            {
                if (a_focus) Magelight->v4()->EnterUIModeEx(a_id,true);
                else Magelight->v4()->ExitUIMode();
            }
            break;
            default:
            break;
        }
    }

    void UIManager::RegisterJSListener(ViewId a_id, string a_name, JsListenerFn a_callback)
    {
        switch(GetUsedUIMod())
        {
            case UIMods::PrismaUI:
            {
                PrismaUI->RegisterJSListener(a_id,a_name.c_str(),a_callback);
            }
            break;
            case UIMods::Magelight:
            {
                Magelight->v4()->RegisterJSListener(a_id,a_name.c_str(),a_callback);
            }
            break;
            default:
            break;
        }
    }

    void UIManager::Invoke(ViewId a_id, string a_js)
    {
        switch(GetUsedUIMod())
        {
            case UIMods::PrismaUI:
            {
                PrismaUI->Invoke(a_id,a_js.c_str());
            }
            break;
            case UIMods::Magelight:
            {
                Magelight->v4()->InvokeJS(a_id,a_js.c_str());
            }
            break;
            default:
            break;
        }
    }

    void UIManager::InteropCall(ViewId a_id, string a_fun, string a_arg)
    {
        switch(GetUsedUIMod())
        {
            case UIMods::PrismaUI:
            {
                PrismaUI->InteropCall(a_id,a_fun.c_str(),a_arg.c_str());
            }
            break;
            case UIMods::Magelight:
            {
                Magelight->v4()->InteropCall(a_id,a_fun.c_str(),a_arg.c_str());
            }
            break;
            default:
            break;
        }
    }

    UIMods UIManager::GetUsedUIMod()
    {
        UIMods loc_res;
        const string loc_pref = Config::GetSingleton()->GetVariable<string>("Ui.sPreferedMod","PrismaUI");
        if (PrismaUI && (boost::iequals(loc_pref,"PrismaUI") || !Magelight))
        {
            loc_res = UIMods::PrismaUI;
        }
        else if (Magelight && (boost::iequals(loc_pref,"Magelight") || !PrismaUI))
        {
            loc_res = UIMods::Magelight;
        }
        else
        {
            loc_res = UIMods::None;
        }
        return loc_res;
    }

}