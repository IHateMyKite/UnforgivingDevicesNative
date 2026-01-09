#pragma once

namespace UD 
{
    struct Module
    {
        RE::BSTSmartPointer<RE::BSScript::Object> object = nullptr;
        RE::TESQuest* quest     = nullptr;

        uint32_t    Priority = 0xFFFFFFFFU;
        std::string Name;
        std::string Alias;
        std::string Description;
        std::vector<RE::TESQuest*> Dependency;
        bool SetupCalled    = false;
        bool SetupDone      = false;
        bool ReloadCalled   = false;
        bool ReloadDone     = false;
        bool QuestStarting  = false;
    };

    class ModuleManager
    {
    SINGLETONHEADER(ModuleManager)
    public:
        void Update(float a_delta);
        void Reload(bool a_setDelay);
        void AddModule(RE::VMHandle a_handle, Module a_module);
        void Clean();
        void SetPapyrusReady();
        void SetDelay(float a_time);
    private:
        void UpdateModuleVariables();
        void CallSetup();
        void CallReload();
        Module* GetModuleByQuest(RE::TESQuest* a_quest);
        bool IsQuestReady(RE::TESQuest* a_quest, bool a_checkreload = false);
        bool AllModulesReady();
        bool AllModulesReloaded();
        void ResetReloaded();
        bool _WaitingForPapyrus = true;
        bool _WaitForReload = false;
        float _Delay = 0.0f;
        mutable std::unordered_map<RE::VMHandle,Module>   _modules;
    };
};