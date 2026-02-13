#pragma once

#include "Settings.h"

class FOVPatch
{
public:
    static bool Install();

private:
    struct UpdateCamera
    {
        static void thunk(RE::TESCamera* a_camera)
        {
            func(a_camera);

            if (!a_camera || !a_camera->currentState) {
                return;
            }

            auto*      settings     = Settings::GetSingleton();
            const bool isFirstPerson = (a_camera->currentState->id == RE::CameraState::kFirstPerson);

            // Determine if we should be actively overriding FOV right now
            const bool shouldOverride = isFirstPerson
                                     && settings->enableFOVOverride
                                     && !IsInDialogue();

            auto* playerCamera = static_cast<RE::PlayerCamera*>(a_camera);
            auto& runtimeData  = playerCamera->GetRuntimeData2();

            if (shouldOverride) {
                // Save the game's original values before we touch anything
                if (!isCurrentlyOverriding) {
                    savedWorldFOV         = runtimeData.worldFOV;
                    savedFirstPersonFOV   = runtimeData.firstPersonFOV;
                    isCurrentlyOverriding = true;
                }

                runtimeData.firstPersonFOV = settings->firstPersonHandsFOV;
                runtimeData.worldFOV       = settings->firstPersonWorldFOV;
            } else if (isCurrentlyOverriding) {
                // We were overriding but should stop — restore the game's original values
                // and get completely out of the way
                runtimeData.worldFOV       = savedWorldFOV;
                runtimeData.firstPersonFOV = savedFirstPersonFOV;
                isCurrentlyOverriding      = false;
            }
            // When not overriding and not restoring, do nothing — let the game
            // and other mods control FOV freely
        }

        static bool IsInDialogue()
        {
            if (const auto ui = RE::UI::GetSingleton()) {
                return ui->IsMenuOpen(RE::DialogueMenu::MENU_NAME);
            }
            return false;
        }

        static inline REL::Relocation<decltype(thunk)> func;
        static inline bool  isCurrentlyOverriding{ false };
        static inline float savedWorldFOV{ 0.0f };
        static inline float savedFirstPersonFOV{ 0.0f };
    };
};
