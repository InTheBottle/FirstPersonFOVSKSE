#pragma once

#include "Settings.h"

class MenuZoomPatch
{
public:
    static bool Install()
    {
        logger::info("Installing MenuZoomPatch...");

        // Hook TESCamera::SetState to block kTween camera state transitions
        // This prevents the zoom-out-to-third-person effect when opening menus
        // TESCamera::SetState = RELOCATION_ID(32290, 33026)
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
