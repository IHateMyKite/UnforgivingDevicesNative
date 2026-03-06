#pragma once
#include <UD_ModuleManager.h>
#include <UD_Utility.h>
#include <UD_Lua.h>

namespace UD
{
    struct FunctionDetails
    {
        std::string Owner;
        std::string Name;
    };

    enum class FuncArgTypes
    {
        eNone,
        eInt,
        eFloat,
        eBool,
        eString,
        eForm,
        eActor,
        eAlias
    };

    union FuncArgVal
    {
        FuncArgVal(){uInt = 0U;};
        ~FuncArgVal(){};
        int uInt;
        float uFloat;
        bool uBool;
        std::string uString;
        RE::TESForm* uForm;
        RE::Actor* uActor;
        RE::BGSBaseAlias* uAlias;
    };

    struct FuncArg
    {
        FuncArg() : Type(FuncArgTypes::eInt) {Val.uInt = 0;}
        FuncArg(int val) : Type(FuncArgTypes::eInt) {Val.uInt = val;}
        FuncArg(float val) : Type(FuncArgTypes::eFloat) {Val.uFloat = val;}
        FuncArg(bool val) : Type(FuncArgTypes::eBool) {Val.uBool = val;}
        FuncArg(std::string val) : Type(FuncArgTypes::eString) {Val.uString = val;}
        FuncArg(RE::TESForm* val) : Type(FuncArgTypes::eForm) {Val.uForm = val;}
        FuncArg(RE::Actor* val) : Type(FuncArgTypes::eActor) {Val.uActor = val;}
        FuncArg(RE::BGSBaseAlias* val) : Type(FuncArgTypes::eAlias) {Val.uAlias = val;}
        FuncArgVal   Val;
        FuncArgTypes Type;
    };

    typedef std::shared_ptr<FuncArg> FuncArgPtr;

    template <class... Args>
    class FunctionArguments2 : public RE::BSScript::IFunctionArguments
    {
    private:
        using super = IFunctionArguments;

    public:
        ~FunctionArguments2() override = default;  // 00

        void SetArgs(std::vector<FuncArgPtr> a_args) {_args2 = a_args;}

        bool operator()(RE::BSScrapArray<Variable>& a_dst) const override  // 01
        {
            a_dst.resize(_args2.size());
            for (size_t i = 0; i < a_dst.size(); i++)
            {
                switch(_args2[i]->Type)
                {
                    case FuncArgTypes::eInt:
                        a_dst[i].Pack(_args2[i]->Val.uInt);
                    break;
                    case FuncArgTypes::eFloat:
                        a_dst[i].Pack(_args2[i]->Val.uFloat);
                    break;
                    case FuncArgTypes::eBool:
                        a_dst[i].Pack(_args2[i]->Val.uBool);
                    break;
                    case FuncArgTypes::eString:
                        a_dst[i].Pack(_args2[i]->Val.uString);
                    break;
                    case FuncArgTypes::eActor:
                        a_dst[i].Pack(_args2[i]->Val.uActor);
                    break;
                    case FuncArgTypes::eAlias:
                        a_dst[i].Pack(_args2[i]->Val.uAlias);
                    break;
                    case FuncArgTypes::eForm:
                        a_dst[i].Pack(_args2[i]->Val.uForm);
                    break;
                    default:
                        ERROR("Unsupported argument type")
                    break;
                }
                //DEBUG("Parsed {} arguments",a_dst.size())
            }
            return true;
        }
    private:
        std::vector<FuncArgPtr> _args2;
    };

    class LuaPapyrusCallback : public RE::BSScript::IStackCallbackFunctor
    {
    public:
        LuaPapyrusCallback() : IStackCallbackFunctor() {}
        LuaPapyrusCallback(int a_id,std::string callback) : IStackCallbackFunctor() , _minigameId(a_id),_callback(callback) {}
        void operator()(Variable a_result);
        void SetObject(const Object& a_object) {};
    private:
        int             _minigameId = 0;
        std::string     _callback = "";
        VariableValue   _var;
        bool            _callbackcalled = false;
    };


    struct PapFunc
    {
        int Id;
        std::string             Module;
        Object                  Source;
        std::string             Callback;
        std::string             Callback2;
        std::vector<FuncArgPtr> Args;
    };

    void            CallPapyrusFunc(PapFunc& a_func);
    Object          GetSource(void* a_source, std::string a_type);
    FunctionDetails ParseFunction(std::string a_var);



}