#pragma once

#include "SKSEMenuFramework.h"
#include "Settings.h"

class InGameMenu
{
public:
    static bool Register()
    {
        if (!SKSEMenuFramework::IsInstalled()) {
            logger::info("SKSEMenuFramework not installed, in-game menu disabled.");
            return false;
        }

        SKSEMenuFramework::SetSection("FirstPersonFOV");
        SKSEMenuFramework::AddSectionItem("Settings", RenderMenu);

        logger::info("In-game menu registered with SKSEMenuFramework.");
        return true;
    }

private:
    static void __stdcall RenderMenu()
    {
        auto* settings = Settings::GetSingleton();

        ImGuiMCP::SeparatorText("First Person FOV");

        ImGuiMCP::Checkbox("Enable FOV Override", &settings->enableFOVOverride);

        if (settings->enableFOVOverride) {
            ImGuiMCP::SliderFloat("Hands FOV", &settings->firstPersonHandsFOV, 1.0f, 165.0f, "%.0f");
            ImGuiMCP::TextDisabled("Controls how close your hands/arms appear.");
            ImGuiMCP::TextDisabled("Lower = hands closer. Default: 80");

            ImGuiMCP::Spacing();

            ImGuiMCP::SliderFloat("World FOV", &settings->firstPersonWorldFOV, 1.0f, 165.0f, "%.0f");
            ImGuiMCP::TextDisabled("World field of view in first person. Default: 80");
        }

        ImGuiMCP::Spacing();
        ImGuiMCP::Separator();
        ImGuiMCP::Spacing();

        ImGuiMCP::SeparatorText("Menu Settings");

        ImGuiMCP::Checkbox("Disable Menu Zoom", &settings->disableMenuZoom);
        ImGuiMCP::TextDisabled("Removes the zoom/freeze-frame effect when opening menus.");
        ImGuiMCP::TextDisabled("Takes effect the next time a menu is opened.");

        ImGuiMCP::Spacing();
        ImGuiMCP::Separator();
        ImGuiMCP::Spacing();

        if (ImGuiMCP::Button("Save Settings")) {
            settings->Save();
            savedTimer = 3.0f;
        }

        ImGuiMCP::SameLine();

        if (ImGuiMCP::Button("Reset to Defaults")) {
            settings->ResetDefaults();
        }

        if (savedTimer > 0.0f) {
            ImGuiMCP::SameLine();
            ImGuiMCP::TextColored({ 0.0f, 1.0f, 0.0f, 1.0f }, "Saved!");
            savedTimer -= 1.0f / 60.0f;
        }
    }

    static inline float savedTimer{ 0.0f };
};
