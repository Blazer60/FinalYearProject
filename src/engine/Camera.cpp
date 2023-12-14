/**
 * @file Camera.cpp
 * @author Ryan Purse
 * @date 13/12/2023
 */


#include "Camera.h"

#include "Actor.h"
#include "BloomPass.h"
#include "ColourGrading.h"
#include "Core.h"
#include "EngineState.h"
#include "GraphicsState.h"

namespace engine
{
    Camera::Camera()
    {
        mPostProcessStack.emplace_back(std::make_unique<BloomPass>());
        mPostProcessStack.emplace_back(std::make_unique<ColourGrading>());
    }

    void Camera::onUpdate()
    {
        mViewMatrix = glm::inverse(mActor->getTransform());
    }

    void Camera::onDrawUi()
    {
        ImGui::PushID("CameraSettings");
        if (ImGui::TreeNodeEx("Camera", ImGuiTreeNodeFlags_Framed | ImGuiTreeNodeFlags_SpanFullWidth | ImGuiTreeNodeFlags_DefaultOpen))
        {
            if (ImGui::Button("Destroy Component"))
                mActor->removeComponent(this);

            ImGui::Checkbox("Main Camera?", &mIsMainCamera);
            ImGui::DragFloat("FOV", &mFov);
            ImGui::DragFloat("Near Clip Distance", &mNearClip);
            ImGui::DragFloat("Far Clip Distance", &mFarClip);
            ImGui::DragFloat("EV100", &mEv100);

            ImGui::TreePop();
        }

        ImGui::PopID();
    }

    bool Camera::isMainCamera() const
    {
        return mIsMainCamera;
    }

    CameraSettings Camera::toCameraSettings()
    {
        return {
            mFov,
            mNearClip,
            mFarClip,
            mViewMatrix,
            mEv100,
            mPostProcessStack
        };
    }

    glm::mat4 Camera::getViewMatrix() const
    {
        return mViewMatrix;
    }
}
