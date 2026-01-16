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
        bool SetupCalled2   = false; //For some reason, sometimes the main variable is not updated
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
        bool IsReady(bool a_CheckReload);
        RE::TESQuest* GetModuleByAlias(std::string a_alias);
        std::vector<RE::TESQuest*> GetModules();
        std::vector<RE::TESQuest*> GetModuleDependency(RE::TESQuest* a_module);
        std::vector<RE::TESQuest*> GetDependantModules(RE::TESQuest* a_module);
        void ResetModule(RE::TESQuest* a_module);
        std::vector<RE::TESQuest*> GetModulesByScript(std::string a_script);
        std::vector<RE::BGSBaseAlias*> GetModulesAliasesByScript(std::string a_script);
    private:
        void UpdateModuleVariables();
        int CallSetup();
        int CallReload();
        std::vector<Module*> GetSortedModuleList();
        Module* GetModuleByQuest(RE::TESQuest* a_quest);
        bool IsQuestReady(RE::TESQuest* a_quest, bool a_checkreload = false);
        bool AllModulesReady();
        bool AllModulesReloaded();
        void ResetReloaded();
        bool _WaitingForPapyrus = true;
        bool _WaitForReload = false;
        float _Delay = 0.0f;
        float _MessageTimer = 0.0f;
        bool _ReloadMessagePrinted  = false;
        bool _SetupMessagePrinted   = false;
        mutable std::unordered_map<RE::VMHandle,Module>   _modules;
    };

    inline bool AreModulesReady(PAPYRUSFUNCHANDLE,bool a_CheckReload)
    {
        return ModuleManager::GetSingleton()->IsReady(a_CheckReload);
    }

    inline RE::TESQuest* GetModuleByAlias(PAPYRUSFUNCHANDLE, std::string a_alias)
    {
        return ModuleManager::GetSingleton()->GetModuleByAlias(a_alias);
    }

    inline std::vector<RE::TESQuest*> GetModules(PAPYRUSFUNCHANDLE)
    {
        return ModuleManager::GetSingleton()->GetModules();
    }

    inline std::vector<RE::TESQuest*> GetModuleDependency(PAPYRUSFUNCHANDLE, RE::TESQuest* a_Module)
    {
        return ModuleManager::GetSingleton()->GetModuleDependency(a_Module);
    }

    inline std::vector<RE::TESQuest*> GetDependantModules(PAPYRUSFUNCHANDLE, RE::TESQuest* a_Module)
    {
        return ModuleManager::GetSingleton()->GetDependantModules(a_Module);
    }

    inline void ResetModule(PAPYRUSFUNCHANDLE, RE::TESQuest* a_Module)
    {
        return ModuleManager::GetSingleton()->ResetModule(a_Module);
    }

    inline std::vector<RE::TESQuest*> GetModulesByScript(PAPYRUSFUNCHANDLE, std::string a_script)
    {
        return ModuleManager::GetSingleton()->GetModulesByScript(a_script);
    }

    inline std::vector<RE::BGSBaseAlias*> GetModulesAliasesByScript(PAPYRUSFUNCHANDLE, std::string a_script)
    {
        return ModuleManager::GetSingleton()->GetModulesAliasesByScript(a_script);
    }
};