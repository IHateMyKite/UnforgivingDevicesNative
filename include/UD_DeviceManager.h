#pragma once

namespace UD
{
    using Object = RE::BSTSmartPointer<RE::BSScript::Object>;
    enum class DeviceConfigStatus : uint8_t
    {
        sOK             = 0U,
        sDisabled       = 1U,
        sMissingMaster  = 2U,
        sError          = 3U
    };

    enum class DeviceVariableType
    {
        eNone = 0,
        eInt,
        eFloat,
        eString,
        eBool,
        eObject,
        eIntArray,
        eFloatArray,
        eStringArray,
        eBoolArray,
        eObjectArray,
        eMax
    };

    struct DeviceVariable
    {
        std::string name;
        std::string nameDocu;
        DeviceVariableType type = DeviceVariableType::eNone;
        bool property = true;
        std::string GetValue(Object a_device) const;
    };

    struct DeviceConfig
    {
        std::string name;
        std::string description;
        std::string script;
        std::vector<DeviceVariable> variables;
        /* TODO: Minigames */
    };

    struct DeviceConfigJson
    {
        std::shared_ptr<boost::property_tree::ptree> json;
        DeviceConfigStatus status;
        std::string error;
        DeviceConfig config;
        void InitConfig();
    };

    class DeviceManager
    {
    SINGLETONHEADER(DeviceManager)
    public:
        void Reload();

    private:
        bool _init = false;
        std::unordered_map<std::string,std::shared_ptr<DeviceConfigJson>> _jsoncache;
    };

}