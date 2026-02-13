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

            auto*      settings      = Settings::GetSingleton();
            const bool isFirstPerson  = (a_camera->currentState->id == RE::CameraState::kFirstPerson);

            auto* playerCamera = static_cast<RE::PlayerCamera*>(a_camera);
            auto& runtimeData  = playerCamera->GetRuntimeData2();

            if (isFirstPerson && settings->enableFOVOverride) {
                if (!wasInFirstPerson) {
                    savedWorldFOV       = runtimeData.worldFOV;
                    savedFirstPersonFOV = runtimeData.firstPersonFOV;
                }

                if (!IsInDialogue()) {
                    runtimeData.firstPersonFOV = settings->firstPersonHandsFOV;
                    runtimeData.worldFOV       = settings->firstPersonWorldFOV;
                    isCurrentlyOverriding = true;
                } else if (isCurrentlyOverriding) {
                    runtimeData.worldFOV       = savedWorldFOV;
                    runtimeData.firstPersonFOV = savedFirstPersonFOV;
                    isCurrentlyOverriding      = false;
                }
            } else if (isCurrentlyOverriding) {
                runtimeData.worldFOV       = savedWorldFOV;
                runtimeData.firstPersonFOV = savedFirstPersonFOV;
                isCurrentlyOverriding      = false;
            }

            wasInFirstPerson = isFirstPerson;
        }

        static bool IsInDialogue()
        {
            if (const auto ui = RE::UI::GetSingleton()) {
                return ui->IsMenuOpen(RE::DialogueMenu::MENU_NAME);
            }
            return false;
        }

        static inline REL::Relocation<decltype(thunk)> func;
        static inline bool  wasInFirstPerson{ false };
        static inline bool  isCurrentlyOverriding{ false };
        static inline float savedWorldFOV{ 0.0f };
        static inline float savedFirstPersonFOV{ 0.0f };
    };
};
