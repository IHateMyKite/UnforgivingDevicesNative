#include <UD_H.h>
#include <UD_Utility.h>
#include <UD_Config.h>
#include <UD_Serialization.h>
#include <UD_RegisterPapyrus.h>
#include <UD_GameEvents.h>
#include <OrgasmSystem/OrgasmConfig.h>

namespace logger = SKSE::log;

#include <spdlog/sinks/basic_file_sink.h>

#pragma message(__FILE__)

void SetupLog() {
    auto logsFolder = SKSE::log::log_directory();
    if (!logsFolder) SKSE::stl::report_and_fail("SKSE log_directory not provided, logs disabled.");
    auto pluginName = SKSE::PluginDeclaration::GetSingleton()->GetName();
    auto logFilePath = *logsFolder / std::format("{}.log", pluginName);
    auto fileLoggerPtr = std::make_shared<spdlog::sinks::basic_file_sink_mt>(logFilePath.string(), true);
    auto loggerPtr = std::make_shared<spdlog::logger>("log", std::move(fileLoggerPtr));
    spdlog::set_default_logger(std::move(loggerPtr));
    spdlog::set_level(spdlog::level::trace);
    spdlog::flush_on(spdlog::level::trace);
}

void InitializeSerialization() {
    LOG("Initializing cosave serialization...");
    auto* serde = SKSE::GetSerializationInterface();
    serde->SetUniqueID(_byteswap_ulong('UDNP'));
    serde->SetSaveCallback(UD::OnGameSaved);
    serde->SetRevertCallback(UD::OnRevert);
    serde->SetLoadCallback(UD::OnGameLoaded);
    LOG("Cosave serialization initialized.");
}

SKSEPluginLoad(const SKSE::LoadInterface *skse) {
    SKSE::Init(skse);
    SetupLog();

    UD::Config::GetSingleton()->Setup();
    ORS::Config::GetSingleton()->Setup();

    SKSE::GetPapyrusInterface()->Register(UD::RegisterPapyrusFunctions);
    SKSE::GetMessagingInterface()->RegisterListener(UD::OnMessageReceived);
    InitializeSerialization();
    SKSE::AllocTrampoline(64);
    return true;
}