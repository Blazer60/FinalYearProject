/**
 * @file MainCamera.h
 * @author Ryan Purse
 * @date 13/02/2022
 */


#pragma once

#include "Pch.h"
#include "gtc/quaternion.hpp"
#include "glfw3.h"
#include "CameraSettings.h"
#include "PostProcessLayer.h"
#include "Drawable.h"

/**
 * A basic main camera so that we can move around the world.
 * @author Ryan Purse
 * @date 13/02/2022
 */
class MainCamera
    : public ui::Drawable
{
public:
    explicit MainCamera(const glm::vec3 &position);
    MainCamera();
    ~MainCamera() override;
    
    void init();
    void update();
    void move();
    
    void setProjectionMatrix(glm::vec2 viewSize);
    
    [[nodiscard]] const glm::mat4 &getVpMatrix() const;
    
    [[nodiscard]] const glm::mat4 &getViewMatrix() const;
    
    [[nodiscard]] const glm::vec3 &getPosition() const;
    
    [[nodiscard]] glm::mat4 getProjectionMatrix() const;
    
    [[nodiscard]] CameraSettings toSettings();
    
    glm::vec3 getEndOfBoomArmPosition() const;

protected:
    glm::mat4       mVpMatrix           { 1.f };
    glm::mat4       mViewMatrix         { 1.f };
    glm::mat4       mProjectionMatrix   { 1.f };
    
    glm::vec3       mPosition           { 0.f, 0.f, 3.f };
    glm::quat       mRotation           { glm::vec3(0.f) };
    
    void onDrawUi() override;

public:
    [[nodiscard]] const glm::quat &getRotation() const;

protected:
    glm::dvec2      mPanAngles          { 0.f };
    
    float           mSpeed              { 30.f };
    float           mMouseSpeed         { 0.5f };
    
    float           mFovY               { glm::radians(45.f) };
    float           mNearClip           { 0.01f };
    float           mFarClip            { 1000.f };
    float           mEV100              { 10.f };
    
    std::vector<std::unique_ptr<PostProcessLayer>> mPostProcessStack;
    
    uint32_t mFocusActorEventToken { 0 };
    uint32_t mOrbitEventToken { 0 };
    uint32_t mMoveForwardEventToken { 0 };
    uint32_t mMoveRightEventToken { 0 };
    uint32_t mMoveUpEventToken { 0 };
    uint32_t mDoMoveToken { 0 };
    uint32_t mZoomViewportToken { 0 };
    uint32_t mZoomThirdPersonToken { 0 };
    
    glm::vec3 mInputDirection { 0.f };
    bool mDoMoveAction { false };
    
    float mRotationSpeed { 0.1f };
    float mCameraBoomDistance { 30.f };
    float mCameraBoomDelta { 2.f };
    float mCameraBoomMin { 0.5f };
    
    void moveFirstPerson();
    void rotateThirdPerson();
    
    void gotoSelectedActor();
    
    void zoomCamera(float zoomDistance);
};


