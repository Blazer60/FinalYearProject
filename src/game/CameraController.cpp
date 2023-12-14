/**
 * @file CameraController.cpp
 * @author Ryan Purse
 * @date 14/12/2023
 */


#include "CameraController.h"

#include "Actor.h"
#include "EngineState.h"
#include "GameInput.h"

CameraController::CameraController()
{
    mMoveForwardToken = gameInput->onMoveForward.subscribe([this](const float value) { mInputDirection.y = value; });
    mMoveRightToken = gameInput->onMoveRight.subscribe([this](const float value) { mInputDirection.x = value; });
}

CameraController::~CameraController()
{
    gameInput->onMoveForward.unSubscribe(mMoveForwardToken);
    gameInput->onMoveRight.unSubscribe(mMoveRightToken);
}

void CameraController::onBegin()
{

}

void CameraController::onUpdate()
{
    const auto timestep = timers::deltaTime<float>();

    const glm::vec2 mouseOffset = engine::eventHandler->getMouseDelta();
    mPanAngles -= glm::radians(mouseOffset) * 0.5f;
    mActor->rotation = glm::angleAxis(static_cast<float>(mPanAngles.x), glm::vec3(0.f, 1.f, 0.f))
                     * glm::angleAxis(static_cast<float>(mPanAngles.y), glm::vec3(1.f, 0.f, 0.f));

    if (glm::length(mInputDirection) > 0.f)
        glm::normalize(mInputDirection);

    mActor->position += mActor->rotation * (1.f * timestep * glm::vec3(mInputDirection.x, 0.f, mInputDirection.y));

    mInputDirection = glm::vec2(0.f);
}

void CameraController::onDrawUi()
{
    ImGui::PushID("CameraController");
    if (ImGui::TreeNodeEx("Camera Controller", ImGuiTreeNodeFlags_Framed | ImGuiTreeNodeFlags_SpanFullWidth | ImGuiTreeNodeFlags_DefaultOpen))
    {
        if (ImGui::Button("Destroy Component"))
            mActor->removeComponent(this);

        ImGui::TreePop();
    }

    ImGui::PopID();
}

void serializeComponent(YAML::Emitter &out, CameraController *cameraController)
{
    out << YAML::Key << "Component" << YAML::Value << "CameraController";
}

