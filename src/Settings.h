#pragma once

class Settings : public Singleton<Settings>
{
public:
    // Detect dialogue-camera mods at runtime. Extend this list if needed.
    static bool DetectDialogueCameraMod()
    {
        static constexpr const wchar_t* kKnownMods[] = {
            L"AlternateConversationCamera.dll",
            L"AlternateConversationCameraPlus.dll",
        };
        for (auto* name : kKnownMods) {
            if (GetModuleHandleW(name)) {
                return true;
            }
        }
        return false;
    }

    void Load()
    {
        constexpr auto defaultPath = L"Data/SKSE/Plugins/FirstPersonFOV.ini";

        CSimpleIniA ini;
        ini.SetUnicode();
        ini.LoadFile(defaultPath);

        enableFOVOverride    = ini.GetBoolValue("FOV", "bEnableFOVOverride", true);
        firstPersonHandsFOV  = static_cast<float>(ini.GetDoubleValue("FOV", "fFirstPersonHandsFOV", 80.0));
        firstPersonWorldFOV  = static_cast<float>(ini.GetDoubleValue("FOV", "fFirstPersonWorldFOV", 80.0));

        enableThirdPersonFOVOverride = ini.GetBoolValue("FOV", "bEnableThirdPersonFOVOverride", false);
        thirdPersonWorldFOV  = static_cast<float>(ini.GetDoubleValue("FOV", "fThirdPersonWorldFOV", 80.0));

        dialogueCameraModDetected = DetectDialogueCameraMod();
        disableOverrideInDialogue = ini.GetBoolValue(
            "Compatibility", "bDisableOverrideInDialogue", dialogueCameraModDetected);

        disableMenuZoom      = ini.GetBoolValue("Menus", "bDisableMenuZoom", true);

        logger::info("Settings loaded:");
        logger::info("  FOV Override Enabled: {}", enableFOVOverride);
        logger::info("  Hands FOV: {}", firstPersonHandsFOV);
        logger::info("  World FOV: {}", firstPersonWorldFOV);
        logger::info("  Third Person FOV Override Enabled: {}", enableThirdPersonFOVOverride);
        logger::info("  Third Person World FOV: {}", thirdPersonWorldFOV);
        logger::info("  Dialogue Camera Mod Detected: {}", dialogueCameraModDetected);
        logger::info("  Disable Override In Dialogue: {}", disableOverrideInDialogue);
        logger::info("  Disable Menu Zoom: {}", disableMenuZoom);
    }

    void Save()
    {
        constexpr auto defaultPath = L"Data/SKSE/Plugins/FirstPersonFOV.ini";

        CSimpleIniA ini;
        ini.SetUnicode();

        ini.SetBoolValue("FOV", "bEnableFOVOverride", enableFOVOverride);
        ini.SetDoubleValue("FOV", "fFirstPersonHandsFOV", static_cast<double>(firstPersonHandsFOV));
        ini.SetDoubleValue("FOV", "fFirstPersonWorldFOV", static_cast<double>(firstPersonWorldFOV));

        ini.SetBoolValue("FOV", "bEnableThirdPersonFOVOverride", enableThirdPersonFOVOverride);
        ini.SetDoubleValue("FOV", "fThirdPersonWorldFOV", static_cast<double>(thirdPersonWorldFOV));

        ini.SetBoolValue("Compatibility", "bDisableOverrideInDialogue", disableOverrideInDialogue);

        ini.SetBoolValue("Menus", "bDisableMenuZoom", disableMenuZoom);

        ini.SaveFile(defaultPath);
        logger::info("Settings saved to INI.");
    }

    void ResetDefaults()
    {
        enableFOVOverride   = true;
        firstPersonHandsFOV = 80.0f;
        firstPersonWorldFOV = 80.0f;
        enableThirdPersonFOVOverride = false;
        thirdPersonWorldFOV = 80.0f;
        // Reset respects detected dialogue camera mods so users don't have
        // to re-toggle every time they reset.
        dialogueCameraModDetected = DetectDialogueCameraMod();
        disableOverrideInDialogue = dialogueCameraModDetected;
        disableMenuZoom     = true;
        logger::info("Settings reset to defaults.");
    }

    bool  enableFOVOverride{ true };
    float firstPersonHandsFOV{ 80.0f };   // FOV for hands/arms rendering
    float firstPersonWorldFOV{ 80.0f };   // World FOV when in first person
    bool  enableThirdPersonFOVOverride{ false };
    float thirdPersonWorldFOV{ 80.0f };   // World FOV when in third person
    bool  disableOverrideInDialogue{ false }; // Skip FOV override while DialogueMenu is open (ACC compat)
    bool  dialogueCameraModDetected{ false }; // Runtime-only: true if a known dialogue camera mod DLL is loaded
    bool  disableMenuZoom{ true };        // Remove the zoom/freeze-frame effect when opening menus
};
