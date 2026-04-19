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
            bool onHorse = cameraInHorseState;
            if (auto* player = RE::PlayerCharacter::GetSingleton()) {
                if (player->IsOnMount()) onHorse = true;
            }
            if (onHorse) {
                // On horseback, clear first-person state (no FP override on a horse)
                // but let the third-person FOV override handle worldFOV below so
                // the user's TP FOV is applied smoothly instead of reverting to
                // vanilla.
                wasInFirstPerson = false;
                hasSnapshot      = false;
                if (isCurrentlyOverriding && a_camera) {
                    auto* playerCam = static_cast<RE::PlayerCamera*>(a_camera);
                    auto& rd = playerCam->GetRuntimeData2();
                    rd.firstPersonFOV = savedFirstPersonFOV;
                    isCurrentlyOverriding = false;
                }
            }

            auto* settings = Settings::GetSingleton();

            // CharacterSheet mod: force worldFOV=50 every frame while open,
            // then restore our settings when it closes.
            if (IsCharacterSheetOpen()) {
                hasSnapshot = false;
                func(a_camera);
                // Force FOV to 50 after the engine update — CharacterSheet
                // expects worldFOV=50 but doesn't set it every frame.
                if (a_camera) {
                    auto* playerCam = static_cast<RE::PlayerCamera*>(a_camera);
                    auto& rd = playerCam->GetRuntimeData2();
                    rd.worldFOV       = 50.0f;
                    rd.firstPersonFOV = 50.0f;
                }
                charSheetWasOpen = true;
                return;
            }

            if (charSheetWasOpen) {
                charSheetWasOpen = false;
                // Restore our saved vanilla values so re-detection below
                // captures correct baselines.
                if (a_camera) {
                    auto* playerCam = static_cast<RE::PlayerCamera*>(a_camera);
                    auto& rd = playerCam->GetRuntimeData2();
                    if (isCurrentlyOverriding) {
                        rd.worldFOV       = savedWorldFOV;
                        rd.firstPersonFOV = savedFirstPersonFOV;
                    }
                    if (isCurrentlyOverridingTP) {
                        rd.worldFOV = savedWorldFOV_TP_orig;
                    }
                }
                wasInFirstPerson        = false;
                wasInThirdPerson        = false;
                isCurrentlyOverriding   = false;
                isCurrentlyOverridingTP = false;
                hasSnapshot             = false;
            }

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
            const bool skipForDialogue = settings->disableOverrideInDialogue && IsInDialogue();
            const bool dialogueJustClosed = prevSkipForDialogue && !skipForDialogue;
            prevSkipForDialogue = skipForDialogue;

            if (isFirstPerson && settings->enableFOVOverride && !skipForDialogue) {
                if (!wasInFirstPerson) {
                    savedWorldFOV       = runtimeData.worldFOV;
                    savedFirstPersonFOV = runtimeData.firstPersonFOV;
                }

                if (dialogueJustClosed) {
                    // Capture whatever FOV the dialogue-camera mod left us at
                    // so we can ease back to the user's target instead of
                    // snapping.
                    fpLerpStartHands = runtimeData.firstPersonFOV;
                    fpLerpStartWorld = runtimeData.worldFOV;
                    fpLerpT          = 0.0f;
                }

                if (fpLerpT < 1.0f) {
                    fpLerpT = (std::min)(1.0f, fpLerpT + kDialogueLerpStep);
                    const float s = fpLerpT * fpLerpT * (3.0f - 2.0f * fpLerpT);
                    runtimeData.firstPersonFOV = std::lerp(fpLerpStartHands, settings->firstPersonHandsFOV, s);
                    runtimeData.worldFOV       = std::lerp(fpLerpStartWorld, settings->firstPersonWorldFOV, s);
                } else {
                    runtimeData.firstPersonFOV = settings->firstPersonHandsFOV;
                    runtimeData.worldFOV       = settings->firstPersonWorldFOV;
                }
                isCurrentlyOverriding = true;
            } else if (isCurrentlyOverriding) {
                runtimeData.worldFOV       = savedWorldFOV;
                runtimeData.firstPersonFOV = savedFirstPersonFOV;
                isCurrentlyOverriding      = false;
                fpLerpT                    = 1.0f;
            }

            wasInFirstPerson = isFirstPerson;

            // --- Third-person FOV override ---
            // Also applies while on horseback (kMount) so the FOV stays smooth
            // between on-foot TP and mounted camera.
            const bool isThirdPerson =
                (a_camera->currentState->id == RE::CameraState::kThirdPerson) ||
                (a_camera->currentState->id == RE::CameraState::kMount) ||
                onHorse;

            if (isThirdPerson && settings->enableThirdPersonFOVOverride && !skipForDialogue) {
                if (!wasInThirdPerson) {
                    savedWorldFOV_TP_orig = runtimeData.worldFOV;
                }
                if (dialogueJustClosed) {
                    tpLerpStartWorld = runtimeData.worldFOV;
                    tpLerpT          = 0.0f;
                }
                if (tpLerpT < 1.0f) {
                    tpLerpT = (std::min)(1.0f, tpLerpT + kDialogueLerpStep);
                    const float s = tpLerpT * tpLerpT * (3.0f - 2.0f * tpLerpT);
                    runtimeData.worldFOV = std::lerp(tpLerpStartWorld, settings->thirdPersonWorldFOV, s);
                } else {
                    runtimeData.worldFOV = settings->thirdPersonWorldFOV;
                }
                isCurrentlyOverridingTP = true;
            } else if (isCurrentlyOverridingTP) {
                runtimeData.worldFOV       = savedWorldFOV_TP_orig;
                isCurrentlyOverridingTP    = false;
                tpLerpT                    = 1.0f;
            }

            wasInThirdPerson = isThirdPerson;
        }

        static bool IsInDialogue()
        {
            if (const auto ui = RE::UI::GetSingleton()) {
                return ui->IsMenuOpen(RE::DialogueMenu::MENU_NAME);
            }
            return false;
        }

        static bool IsCharacterSheetOpen()
        {
            if (const auto ui = RE::UI::GetSingleton()) {
                return ui->IsMenuOpen("CharacterSheet");
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

        // Third-person FOV state
        static inline bool  wasInThirdPerson{ false };
        static inline bool  isCurrentlyOverridingTP{ false };
        static inline float savedWorldFOV_TP_orig{ 0.0f };

        // Dialogue-exit smoothing (ACC compatibility)
        static constexpr float kDialogueLerpStep{ 1.0f / 30.0f }; // ~0.5s at 60fps
        static inline bool  prevSkipForDialogue{ false };
        static inline float fpLerpT{ 1.0f };          // 1.0 = inactive
        static inline float fpLerpStartHands{ 0.0f };
        static inline float fpLerpStartWorld{ 0.0f };
        static inline float tpLerpT{ 1.0f };
        static inline float tpLerpStartWorld{ 0.0f };

        // CharacterSheet tracking
        static inline bool charSheetWasOpen{ false };

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
