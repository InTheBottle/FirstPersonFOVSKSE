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
            // Call the original TESCamera::Update first
            func(a_camera);

            if (!a_camera || !a_camera->currentState) {
                return;
            }

            // Check camera state before accessing settings to avoid unnecessary work
            if (a_camera->currentState->id != RE::CameraState::kFirstPerson) {
                return;
            }

            auto* playerCamera = static_cast<RE::PlayerCamera*>(a_camera);
            auto& runtimeData  = playerCamera->GetRuntimeData2();
            auto* settings     = Settings::GetSingleton();

            if (settings->enableFOVOverride) {
                // Set hands/arms FOV (controls how close hands appear)
                runtimeData.firstPersonFOV = settings->firstPersonHandsFOV;

                // Set world FOV when in first person
                runtimeData.worldFOV = settings->firstPersonWorldFOV;
            } else if (wasOverriding) {
                // Restore default FOV when override is toggled off
                runtimeData.firstPersonFOV = defaultFirstPersonFOV;
                runtimeData.worldFOV = defaultWorldFOV;
                wasOverriding = false;
            }

            if (settings->enableFOVOverride && !wasOverriding) {
                // Capture defaults before first override
                defaultFirstPersonFOV = runtimeData.firstPersonFOV;
                defaultWorldFOV = runtimeData.worldFOV;
                wasOverriding = true;
            }
        }

        static inline REL::Relocation<decltype(thunk)> func;
        static inline bool  wasOverriding{ false };
        static inline float defaultFirstPersonFOV{ 80.0f };
        static inline float defaultWorldFOV{ 80.0f };
    };
};
