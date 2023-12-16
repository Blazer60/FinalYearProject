/**
 * @file CameraController.h
 * @author Ryan Purse
 * @date 14/12/2023
 */


#pragma once

#include <Engine.h>

/**
 * @author Ryan Purse
 * @date 14/12/2023
 */
class CameraController
    : public engine::Component
{
public:
    CameraController();
    ~CameraController() override;

    void onBegin() override;
    void onUpdate() override;
    void onDrawUi() override;

protected:
    float mSpeed { 1.f };
    float mJumpForce { 100.f };
    Ref<engine::RigidBody> mRigidBody;
    engine::vec2 mInputDirection { 0.f };
    bool mWantsJump { false };
    engine::dvec2 mPanAngles { 0.f };

    uint32_t mMoveForwardToken { 0 };
    uint32_t mMoveRightToken { 0 };
    uint32_t mJumpToken { 0 };
    Ref<engine::Actor> mCamera;

    friend void initComponentsForEngine();
    SERIALIZABLE_COMPONENT(CameraController);
};
