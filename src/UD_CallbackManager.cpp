#include <UD_CallbackManager.h>
#include <UD_MinigameManager.h>

void UD::CallPapyrusFunc(PapFunc& a_func)
{
    //DEBUG("CallPapyrusFunc({},0x{:016X},{},{}) called",a_func.Module,(uintptr_t)a_func.Source.get(),a_func.Callback,a_func.Args.size())

    const auto loc_vm = InternalVM::GetSingleton();

    RE::BSTSmartPointer<LuaPapyrusCallback> loc_callback(new LuaPapyrusCallback(a_func.Id,a_func.Callback2));
    RE::BSTSmartPointer<RE::BSScript::IStackCallbackFunctor> loc_callback2 = loc_callback;
    if (a_func.Args.size() > 0)
    {
        auto loc_args = new FunctionArguments2();
        loc_args->SetArgs(a_func.Args);
        loc_vm->DispatchMethodCall(a_func.Source,a_func.Callback,loc_args,loc_callback2);
    }
    else
    {
        RE::MakeFunctionArguments();
        loc_vm->DispatchMethodCall(a_func.Source,a_func.Callback,RE::MakeFunctionArguments(),loc_callback2);
    }
}

Object UD::GetSource(void* a_source, std::string a_type)
{
    if (a_type == "thisdevice" && a_source)
    {
        ObjectPtr* loc_device = reinterpret_cast<ObjectPtr*>(a_source);
        return Object(loc_device);
    }
    else
    {
        auto loc_module = ModuleManager::GetSingleton()->GetModuleObjectByAlias(a_type);
        return loc_module ? loc_module->object : Object();
    }
    return Object();
}

UD::FunctionDetails UD::ParseFunction(std::string a_var)
{
    static const std::regex loc_ParseRegex(R"((?:(\w+)::([\w_]+))*)");
    FunctionDetails loc_res;

    loc_res.Owner   = std::regex_replace(a_var, loc_ParseRegex, "$1");
    std::transform(loc_res.Owner.begin(), loc_res.Owner.end(), loc_res.Owner.begin(), ::tolower);
    loc_res.Name    = std::regex_replace(a_var, loc_ParseRegex, "$2");

    return loc_res;
}

UD::LuaPapyrusCallback::~LuaPapyrusCallback()
{
}

void UD::LuaPapyrusCallback::operator()(Variable a_result)
{
    if (_callback != "" && !_callbackcalled)
    {
        _var = ParsePapVar(&a_result);
        MinigameManager::GetSingleton()->SendPapCallback(_minigameId,_callback,_var);
    }
    _callbackcalled = true;
}

void UD::LuaPapyrusCallback::SetObject(const Object& a_object)
{
}
