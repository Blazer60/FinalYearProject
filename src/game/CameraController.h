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
    glm::vec2 mInputDirection { 0.f };
    glm::dvec2 mPanAngles { 0.f };
    uint32_t mMoveForwardToken { 0 };
    uint32_t mMoveRightToken { 0 };

    friend void initComponentsForEngine();
    SERIALIZABLE_COMPONENT(CameraController);
};
