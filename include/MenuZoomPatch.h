#pragma once

#include "Settings.h"

class MenuZoomPatch
{
public:
    static bool Install()
    {
        logger::info("Installing MenuZoomPatch...");

        auto setStateAddr = REL::RelocationID(32290, 33026).address();

        SKSE::AllocTrampoline(14);
        auto& trampoline = SKSE::GetTrampoline();

        originalSetState = trampoline.write_branch<5>(setStateAddr, HookedSetState);

        logger::info("MenuZoomPatch: Hooked TESCamera::SetState at 0x{:X}.", setStateAddr);
        return true;
    }

private:
    static void HookedSetState(RE::TESCamera* a_camera, RE::TESCameraState* a_state)
    {
        auto* settings = Settings::GetSingleton();

        if (settings->disableMenuZoom && a_camera && a_state) {
            const bool cameraInHorseState = a_camera->currentState &&
                a_camera->currentState->id == RE::CameraState::kMount;
            if (auto* player = RE::PlayerCharacter::GetSingleton();
                player && (player->IsOnMount() || cameraInHorseState)) {
                originalSetState(a_camera, a_state);
                return;
            }

            auto* playerCamera = RE::PlayerCamera::GetSingleton();
            if (playerCamera && a_camera == static_cast<RE::TESCamera*>(playerCamera)) {
                auto& runtimeData = playerCamera->GetRuntimeData();
                auto* tweenState = runtimeData.cameraStates[RE::CameraState::kTween].get();
                if (a_state == tweenState) {
                    logger::debug("MenuZoomPatch: Blocked kTween camera state transition");
                    return;
                }
            }
        }

        originalSetState(a_camera, a_state);
    }

    static inline REL::Relocation<decltype(HookedSetState)*> originalSetState;
};
