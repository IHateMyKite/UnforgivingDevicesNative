#pragma once

namespace UD
{
    //using Object = RE::BSTSmartPointer<RE::BSScript::Object>;
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
        std::string atributesRaw;
        std::unordered_map<std::string,std::optional<std::string>> atributes;
        DeviceVariableType type = DeviceVariableType::eNone;
        bool property = true;
    };

    struct DeviceConfig
    {
        std::unordered_map<std::string,std::string> docustr;
        std::string name;
        std::string script;
        std::string description;
        std::vector<DeviceVariable> variables;
        /* TODO: Minigames */
    };

    struct DeviceConfigJson
    {
        std::shared_ptr<boost::property_tree::ptree> json;
        DeviceConfigStatus status;
        std::string error;
        DeviceConfig config;
    };

    class DeviceManager
    {
    SINGLETONHEADER(DeviceManager)
    public:
        void Reload();

        std::vector<DeviceConfig> GetDeviceConfigs(Object a_device);

        float GetDeviceAccessibility(RE::TESObjectARMO* a_rd, ObjectPtr* a_device, RE::Actor* a_actor, RE::Actor* a_helper);
    private:
        bool _init = false;
        //std::unordered_map<std::string,std::shared_ptr<DeviceConfigJson>> _jsoncache;
        std::unordered_map<std::string,DeviceConfig> _DeviceTypes;
    };

}