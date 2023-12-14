/**
 * @file CameraController.h
 * @author Ryan Purse
 * @date 14/12/2023
 */


#pragma once

#include "Component.h"
#include "Pch.h"



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
    glm::vec2 mInputDirection { 0.f };
    bool mWantsJump { false };
    glm::dvec2 mPanAngles { 0.f };

    uint32_t mMoveForwardToken { 0 };
    uint32_t mMoveRightToken { 0 };
    uint32_t mJumpToken { 0 };
    Ref<engine::Actor> mCamera;

    friend void initComponentsForEngine();
    SERIALIZABLE_COMPONENT(CameraController);
};
