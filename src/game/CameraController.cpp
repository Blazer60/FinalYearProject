/**
 * @file CameraController.cpp
 * @author Ryan Purse
 * @date 14/12/2023
 */


#include "CameraController.h"

#include "GameInput.h"
#include <Engine.h>

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

    mCamera = engine::core->getScene()->getActor(mActor->getChildren()[0]);
}

void CameraController::onUpdate()
{
    if (!mRigidBody.isValid())
        return;

    const auto timestep = timers::deltaTime<float>();

    if (engine::eventHandler->updateUserEvents)
    {
        const engine::vec2 mouseOffset = engine::eventHandler->getMouseDelta();
        mPanAngles -= engine::radians(mouseOffset) * 0.5f;
        mCamera->rotation = engine::angleAxis(static_cast<float>(mPanAngles.y), engine::vec3(1.f, 0.f, 0.f));
        mActor->rotation = engine::angleAxis(static_cast<float>(mPanAngles.x), engine::vec3(0.f, 1.f, 0.f));
    }

    if (engine::length(mInputDirection) > 0.f)
        engine::normalize(mInputDirection);

    mRigidBody->active();
    engine::vec3 impulse = mActor->rotation * (mSpeed * timestep * engine::vec3(mInputDirection.x, 0.f, mInputDirection.y));
    if (mWantsJump)
        impulse.y += mJumpForce;
    mRigidBody->addImpulse(impulse);

    mInputDirection = engine::vec2(0.f);
    mWantsJump = false;
}

void CameraController::onDrawUi()
{
    engine::ui::PushID("CameraController");
    if (engine::ui::TreeNodeEx("Camera Controller", ImGuiTreeNodeFlags_Framed | ImGuiTreeNodeFlags_SpanFullWidth | ImGuiTreeNodeFlags_DefaultOpen))
    {
        if (engine::ui::Button("Destroy Component"))
            mActor->removeComponent(this);

        engine::ui::DragFloat("Speed", &mSpeed);
        engine::ui::DragFloat("Jump Force", &mJumpForce);
        engine::ui::TreePop();
    }

    engine::ui::PopID();
}

void serializeComponent(engine::serialize::Emitter &out, CameraController *const cameraController)
{
    out << engine::serialize::Key << "Component" << engine::serialize::Value << "CameraController";
    out << engine::serialize::Key << "Speed" << engine::serialize::Value << cameraController->mSpeed;
    out << engine::serialize::Key << "JumpForce" << engine::serialize::Value << cameraController->mJumpForce;
}

