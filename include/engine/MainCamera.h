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

/**
 * A basic main camera so that we can move around the world.
 * @author Ryan Purse
 * @date 13/02/2022
 */
class MainCamera
{
public:
    explicit MainCamera(const glm::vec3 &position);
    
    MainCamera();
    
    void update();
    
    void imguiUpdate();
    
    void move();
    
    void setProjectionMatrix(glm::vec2 viewSize);
    
    [[nodiscard]] const glm::mat4 &getVpMatrix() const;
    
    [[nodiscard]] const glm::mat4 &getViewMatrix() const;
    
    [[nodiscard]] const glm::vec3 &getPosition() const;
    
    [[nodiscard]] glm::mat4 getProjectionMatrix() const;
    
    CameraSettings toSettings() const;

protected:
    glm::mat4       mVpMatrix           { 1.f };
    glm::mat4       mViewMatrix         { 1.f };
    glm::mat4       mProjectionMatrix   { 1.f };
    
    glm::vec3       mPosition           { 0.f, 0.f, 3.f };
    glm::quat       mRotation           { glm::vec3(0.f) };
public:
    [[nodiscard]] const glm::quat &getRotation() const;

protected:
    glm::dvec2      mPanAngles          { 0.f };
    
    float           mSpeed              { 10.f };
    double          mMouseSpeed         { 0.5f };
    
    float           mFovY               { glm::radians(45.f) };
    float           mNearClip           { 0.001f };
    float           mFarClip            { 100.f };
    
    GLFWwindow *    mWindow             { };
};


