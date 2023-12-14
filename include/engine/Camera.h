/**
 * @file Camera.h
 * @author Ryan Purse
 * @date 13/12/2023
 */


#pragma once

#include "CameraSettings.h"
#include "Component.h"
#include "Pch.h"
#include "PostProcessLayer.h"

namespace engine
{
    /**
     * @author Ryan Purse
     * @date 13/12/2023
     */
    class Camera
        : public Component
    {
    public:
        Camera();
        void onUpdate() override;
        void onDrawUi() override;

        bool isMainCamera() const;
        [[nodiscard]] CameraSettings toCameraSettings();
        [[nodiscard]] glm::mat4 getViewMatrix() const;

    protected:
        std::vector<std::unique_ptr<PostProcessLayer>> mPostProcessStack;
        bool mIsMainCamera { false };
        float mFov { glm::radians(45.f) };
        float mEv100 { 10.f };
        float mNearClip { 0.01f };
        float mFarClip { 1000.f };
        glm::mat4 mViewMatrix { 1.f };

        ENGINE_SERIALIZABLE_COMPONENT(Camera);
    };

} // engine
