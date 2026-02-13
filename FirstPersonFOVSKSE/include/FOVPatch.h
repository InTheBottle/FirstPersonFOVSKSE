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

            auto* settings = Settings::GetSingleton();
            if (!settings->enableFOVOverride) {
                return;
            }

            if (a_camera->currentState->id != RE::CameraState::kFirstPerson) {
                return;
            }

            auto* playerCamera = static_cast<RE::PlayerCamera*>(a_camera);
            auto& runtimeData  = playerCamera->GetRuntimeData2();

            // Set hands/arms FOV (controls how close hands appear)
            runtimeData.firstPersonFOV = settings->firstPersonHandsFOV;

            // Set world FOV when in first person
            runtimeData.worldFOV = settings->firstPersonWorldFOV;
        }

        static inline REL::Relocation<decltype(thunk)> func;
    };
};
