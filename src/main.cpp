#include "FOVPatch.h"
#include "InGameMenu.h"
#include "MenuZoomPatch.h"
#include "Settings.h"

void InitLogger()
{
    auto path{ logger::log_directory() };
    if (!path) {
        return;
    }

    *path /= "FirstPersonFOV.log"sv;

    auto sink = std::make_shared<spdlog::sinks::basic_file_sink_mt>(path->string(), true);
    auto log  = std::make_shared<spdlog::logger>("global log"s, std::move(sink));

    log->set_level(spdlog::level::info);
    log->flush_on(spdlog::level::info);

    spdlog::set_default_logger(std::move(log));
    spdlog::set_pattern("[%H:%M:%S.%e] [%l] %v"s);

    logger::info("FirstPersonFOV v1.0.0 loaded");
}

void OnMessage(SKSE::MessagingInterface::Message* a_msg)
{
    switch (a_msg->type) {
    case SKSE::MessagingInterface::kDataLoaded:
        Settings::GetSingleton()->Load();
        FOVPatch::Install();
        MenuZoomPatch::Install();
        InGameMenu::Register();
        break;
    default:
        break;
    }
}

SKSEPluginLoad(const SKSE::LoadInterface* a_skse)
{
    InitLogger();

    SKSE::Init(a_skse);

    logger::info("Game version: {}", a_skse->RuntimeVersion().string());

    auto messaging = SKSE::GetMessagingInterface();
    if (!messaging->RegisterListener("SKSE", OnMessage)) {
        logger::critical("Failed to register messaging listener!");
        return false;
    }

    logger::info("Plugin loaded successfully, waiting for data...");
    return true;
}

