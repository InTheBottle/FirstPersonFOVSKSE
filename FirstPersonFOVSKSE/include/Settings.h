#pragma once

class Settings : public Singleton<Settings>
{
public:
    void Load()
    {
        constexpr auto defaultPath = L"Data/SKSE/Plugins/FirstPersonFOV.ini";

        CSimpleIniA ini;
        ini.SetUnicode();
        ini.LoadFile(defaultPath);

        enableFOVOverride    = ini.GetBoolValue("FOV", "bEnableFOVOverride", true);
        firstPersonHandsFOV  = static_cast<float>(ini.GetDoubleValue("FOV", "fFirstPersonHandsFOV", 80.0));
        firstPersonWorldFOV  = static_cast<float>(ini.GetDoubleValue("FOV", "fFirstPersonWorldFOV", 80.0));

        logger::info("Settings loaded:");
        logger::info("  FOV Override Enabled: {}", enableFOVOverride);
        logger::info("  Hands FOV: {}", firstPersonHandsFOV);
        logger::info("  World FOV: {}", firstPersonWorldFOV);
    }

    void Save()
    {
        constexpr auto defaultPath = L"Data/SKSE/Plugins/FirstPersonFOV.ini";

        CSimpleIniA ini;
        ini.SetUnicode();

        ini.SetBoolValue("FOV", "bEnableFOVOverride", enableFOVOverride);
        ini.SetDoubleValue("FOV", "fFirstPersonHandsFOV", static_cast<double>(firstPersonHandsFOV));
        ini.SetDoubleValue("FOV", "fFirstPersonWorldFOV", static_cast<double>(firstPersonWorldFOV));

        ini.SaveFile(defaultPath);
        logger::info("Settings saved to INI.");
    }

    void ResetDefaults()
    {
        enableFOVOverride   = true;
        firstPersonHandsFOV = 80.0f;
        firstPersonWorldFOV = 80.0f;
        logger::info("Settings reset to defaults.");
    }

    bool  enableFOVOverride{ true };
    float firstPersonHandsFOV{ 80.0f };   // FOV for hands/arms rendering
    float firstPersonWorldFOV{ 80.0f };   // World FOV when in first person
};
