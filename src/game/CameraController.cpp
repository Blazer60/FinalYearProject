/**
 * @file CameraController.cpp
 * @author Ryan Purse
 * @date 14/12/2023
 */


#include "CameraController.h"

#include "Actor.h"
#include "Core.h"
#include "EngineState.h"
#include "GameInput.h"
#include "RigidBody.h"

CameraController::CameraController()
{
    mMoveForwardToken = gameInput->onMoveForward.subscribe([this](const float value) { mInputDirection.y = value; });
    mMoveRightToken = gameInput->onMoveRight.subscribe([this](const float value) { mInputDirection.x = value; });
    mJumpToken = gameInput->onJump.subscribe([this] { mWantsJump = true; });
}

CameraController::~CameraController()
{
    gameInput->onMoveForward.unSubscribe(mMoveForwardToken);
    gameInput->onMoveRight.unSubscribe(mMoveRightToken);
    gameInput->onJump.unSubscribe(mJumpToken);
}

void CameraController::onBegin()
{
    if (engine::editor->isUsingPlayModeCamera())  // todo: The user should never have to ask the editor for something.
        engine::event::hideMouseCursor();
    mRigidBody = mActor->getComponent<engine::RigidBody>();
    if (mRigidBody.isValid())
    {
        mRigidBody->setAngularFactor(glm::vec3(0.f));
    }
}

void CameraController::onUpdate()
{
    if (!mRigidBody.isValid())
        return;

    const auto timestep = timers::deltaTime<float>();

    if (engine::eventHandler->updateUserEvents)
    {
        const glm::vec2 mouseOffset = engine::eventHandler->getMouseDelta();
        mPanAngles -= glm::radians(mouseOffset) * 0.5f;
        mActor->rotation = glm::angleAxis(static_cast<float>(mPanAngles.x), glm::vec3(0.f, 1.f, 0.f))
                         * glm::angleAxis(static_cast<float>(mPanAngles.y), glm::vec3(1.f, 0.f, 0.f));
    }

    if (glm::length(mInputDirection) > 0.f)
        glm::normalize(mInputDirection);

    mRigidBody->active();
    glm::vec3 impulse = mActor->rotation * (mSpeed * timestep * glm::vec3(mInputDirection.x, 0.f, mInputDirection.y));
    if (mWantsJump)
        impulse.y += 55.f;
    mRigidBody->addImpulse(impulse);

    mInputDirection = glm::vec2(0.f);
    mWantsJump = false;
}

void CameraController::onDrawUi()
{
    ImGui::PushID("CameraController");
    if (ImGui::TreeNodeEx("Camera Controller", ImGuiTreeNodeFlags_Framed | ImGuiTreeNodeFlags_SpanFullWidth | ImGuiTreeNodeFlags_DefaultOpen))
    {
        if (ImGui::Button("Destroy Component"))
            mActor->removeComponent(this);

        ImGui::DragFloat("Speed", &mSpeed);
        ImGui::TreePop();
    }

    ImGui::PopID();
}

void serializeComponent(YAML::Emitter &out, CameraController *cameraController)
{
    out << YAML::Key << "Component" << YAML::Value << "CameraController";
    out << YAML::Key << "Speed" << YAML::Value << cameraController->mSpeed;
}

