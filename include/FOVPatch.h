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
            const bool cameraInHorseState = a_camera && a_camera->currentState &&
                a_camera->currentState->id == RE::CameraState::kMount;
            if (auto* player = RE::PlayerCharacter::GetSingleton()) {
                if (player->IsOnMount() || cameraInHorseState) {
                    // Clean up any active state
                    hasSnapshot = false;
                    wasInFirstPerson = false;
                    if (isCurrentlyOverriding && a_camera) {
                        auto* playerCam = static_cast<RE::PlayerCamera*>(a_camera);
                        auto& rd = playerCam->GetRuntimeData2();
                        rd.worldFOV       = savedWorldFOV;
                        rd.firstPersonFOV = savedFirstPersonFOV;
                        isCurrentlyOverriding = false;
                    }
                    func(a_camera);
                    return;
                }
            }

            auto* settings = Settings::GetSingleton();

            if (a_camera && a_camera->currentState &&
                a_camera->currentState->id == RE::CameraState::kThirdPerson &&
                settings->disableMenuZoom)
            {
                auto* playerCam = static_cast<RE::PlayerCamera*>(a_camera);
                auto* tps = static_cast<RE::ThirdPersonState*>(
                    a_camera->currentState.get());
                const bool tweenMenuOpen = IsTweenMenuOpen();

                if (!tweenMenuOpen) {
                    if (!playerCam->cameraRoot) {
                        func(a_camera);
                        return;
                    }
                    savedCameraRootLocal = playerCam->cameraRoot->local;
                    savedCameraRootWorld = playerCam->cameraRoot->world;
                    savedWorldFOV_TP     = playerCam->GetRuntimeData2().worldFOV;
                    savedPos_TP          = playerCam->GetRuntimeData2().pos;
                    savedTargetZoom      = tps->targetZoomOffset;
                    savedCurrentZoom     = tps->currentZoomOffset;
                    savedSavedZoom       = tps->savedZoomOffset;
                    savedPitchZoom       = tps->pitchZoomOffset;
                    savedPosOffsetExp    = tps->posOffsetExpected;
                    savedPosOffsetAct    = tps->posOffsetActual;
                    savedTranslation     = tps->translation;
                    savedRotation        = tps->rotation;
                    savedTargetYaw       = tps->targetYaw;
                    savedCurrentYaw      = tps->currentYaw;
                    savedFreeRotation    = tps->freeRotation;
                    savedFreeRotEnabled  = tps->freeRotationEnabled;
                    hasSnapshot          = true;

                    func(a_camera);
                } else if (hasSnapshot) {
                    tps->targetZoomOffset  = savedTargetZoom;
                    tps->currentZoomOffset = savedCurrentZoom;
                    tps->savedZoomOffset   = savedSavedZoom;
                    tps->pitchZoomOffset   = savedPitchZoom;
                    tps->posOffsetExpected = savedPosOffsetExp;
                    tps->posOffsetActual   = savedPosOffsetAct;
                    tps->translation       = savedTranslation;
                    tps->rotation          = savedRotation;
                    tps->targetYaw         = savedTargetYaw;
                    tps->currentYaw        = savedCurrentYaw;
                    tps->freeRotation      = savedFreeRotation;
                    tps->freeRotationEnabled = savedFreeRotEnabled;

                    if (playerCam->cameraRoot) {
                        playerCam->cameraRoot->local = savedCameraRootLocal;
                        playerCam->cameraRoot->world = savedCameraRootWorld;
                    }

                    auto& rd2    = playerCam->GetRuntimeData2();
                    rd2.worldFOV = savedWorldFOV_TP;
                    rd2.pos      = savedPos_TP;
                    return; 
                } else {
                    func(a_camera);
                }
            } else {
                hasSnapshot = false;
                func(a_camera);
            }

            if (!a_camera || !a_camera->currentState) {
                return;
            }

            // --- First-person FOV override ---
            auto* playerCam = static_cast<RE::PlayerCamera*>(a_camera);
            auto& runtimeData = playerCam->GetRuntimeData2();
            const bool isFirstPerson = (a_camera->currentState->id == RE::CameraState::kFirstPerson);

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

        static bool IsTweenMenuOpen()
        {
            if (const auto ui = RE::UI::GetSingleton()) {
                return ui->IsMenuOpen(RE::TweenMenu::MENU_NAME);
            }
            return false;
        }

        static inline REL::Relocation<decltype(thunk)> func;

        // First-person FOV state
        static inline bool  wasInFirstPerson{ false };
        static inline bool  isCurrentlyOverriding{ false };
        static inline float savedWorldFOV{ 0.0f };
        static inline float savedFirstPersonFOV{ 0.0f };

        static inline bool hasSnapshot{ false };
        static inline RE::NiTransform  savedCameraRootLocal{};
        static inline RE::NiTransform  savedCameraRootWorld{};
        static inline float            savedWorldFOV_TP{ 0.0f };
        static inline RE::NiPoint3     savedPos_TP{};
        static inline float            savedTargetZoom{ 0.0f };
        static inline float            savedCurrentZoom{ 0.0f };
        static inline float            savedSavedZoom{ 0.0f };
        static inline float            savedPitchZoom{ 0.0f };
        static inline RE::NiPoint3     savedPosOffsetExp{};
        static inline RE::NiPoint3     savedPosOffsetAct{};
        static inline RE::NiPoint3     savedTranslation{};
        static inline RE::NiQuaternion savedRotation{};
        static inline float            savedTargetYaw{ 0.0f };
        static inline float            savedCurrentYaw{ 0.0f };
        static inline RE::NiPoint2     savedFreeRotation{};
        static inline bool             savedFreeRotEnabled{ false };
    };
};
