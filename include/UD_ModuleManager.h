#pragma once

namespace UD 
{
    struct Module
    {
        RE::BSTSmartPointer<RE::BSScript::Object> object = nullptr;
        RE::TESQuest* quest     = nullptr;
        uint32_t PRIORITY = 0xFFFFFFFFU;
        bool _SetupCalled  = false;
        bool _SetupDone    = false;
        bool _Updating  = false;
        bool _Disabled  = false;
    };

    class ModuleManager
    {
    SINGLETONHEADER(ModuleManager)
    public:
        void Setup();
        void Update();
        void AddModule(RE::VMHandle a_handle, Module a_module);
    private:
        void UpdateModuleVariables();
        void CallSetupOrUpdate();
        int _installed = false;
        mutable std::map<RE::VMHandle,Module>   _modules;
    };
};