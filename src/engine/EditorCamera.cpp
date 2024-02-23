/**
 * @file EditorCamera.cpp
 * @author Ryan Purse
 * @date 13/02/2022
 */


#include "EditorCamera.h"
#include "Timers.h"
#include "imgui.h"
#include "WindowHelpers.h"
#include "BloomPass.h"
#include "ColourGrading.h"
#include "Editor.h"
#include "EngineState.h"
#include "EventHandler.h"
#include "ProfileTimer.h"
#include "ui/editor/RendererSettings.h"

EditorCamera::EditorCamera()
{
    init();
}

EditorCamera::EditorCamera(const glm::vec3 &position)
    : mPosition(position)
{
    init();
}

void EditorCamera::init()
{
    const glm::vec2 size = glm::ivec2(window::bufferSize());
    mProjectionMatrix = glm::perspective(mFovY, size.x / size.y, mNearClip, mFarClip);
    
    mPostProcessStack.emplace_back(std::make_unique<BloomPass>());
    mPostProcessStack.emplace_back(std::make_unique<ColourGrading>());
    
    mFocusActorEventToken = engine::eventHandler->editor.viewport.onFocusActor.subscribe([this]() { gotoSelectedActor(); });
    mOrbitEventToken = engine::eventHandler->editor.viewport.thirdPerson.onOrbitCamera.subscribe([this](float value) { rotateThirdPerson(); });
    mDoMoveToken = engine::eventHandler->editor.viewport.firstPerson.onStateChanged.subscribe([this](bool isActive) { mDoMoveAction = isActive; });
    mMoveForwardEventToken = engine::eventHandler->editor.viewport.firstPerson.onMoveForward.subscribe([this](float value) { mInputDirection.z = value; });
    mMoveRightEventToken = engine::eventHandler->editor.viewport.firstPerson.onMoveRight.subscribe([this](float value) { mInputDirection.x = value; });
    mMoveUpEventToken = engine::eventHandler->editor.viewport.firstPerson.onMoveUp.subscribe([this](float value) { mInputDirection.y = value; });
    mZoomViewportToken = engine::eventHandler->editor.viewport.onZoom.subscribe([this](float zoomValue){ zoomCamera(-zoomValue * mCameraBoomDelta); });
    mZoomThirdPersonToken = engine::eventHandler->editor.viewport.thirdPerson.onZoomCamera.subscribe([this](float zoomValue){ zoomCamera(engine::eventHandler->getMouseDelta().y); });
}

EditorCamera::~EditorCamera()
{
    engine::eventHandler->editor.viewport.onFocusActor.unSubscribe(mFocusActorEventToken);
    engine::eventHandler->editor.viewport.thirdPerson.onOrbitCamera.unSubscribe(mOrbitEventToken);
    engine::eventHandler->editor.viewport.firstPerson.onMoveForward.unSubscribe(mMoveForwardEventToken);
    engine::eventHandler->editor.viewport.firstPerson.onMoveRight.unSubscribe(mMoveRightEventToken);
    engine::eventHandler->editor.viewport.firstPerson.onMoveUp.unSubscribe(mMoveUpEventToken);
    engine::eventHandler->editor.viewport.firstPerson.onStateChanged.unSubscribe(mDoMoveToken);
    engine::eventHandler->editor.viewport.onZoom.unSubscribe(mZoomViewportToken);
    engine::eventHandler->editor.viewport.thirdPerson.onZoomCamera.unSubscribe(mZoomThirdPersonToken);
}

void EditorCamera::update()
{
    PROFILE_FUNC();
    move();
    mProjectionMatrix = glm::perspective(mFovY, window::aspectRatio(), mNearClip, mFarClip);
    
    mViewMatrix = glm::translate(glm::mat4(1.f), mPosition)
                * glm::mat4(mRotation);
    mViewMatrix = glm::inverse(mViewMatrix);
    
    mVpMatrix   = mProjectionMatrix * mViewMatrix;
}

void EditorCamera::move()
{
    if (mDoMoveAction)
        moveFirstPerson();
    
    mInputDirection = glm::vec3(0.f);
}

void EditorCamera::zoomCamera(float distance)
{
    const float boomDistance = mCameraBoomDistance;
    mCameraBoomDistance = glm::max(mCameraBoomMin, mCameraBoomDistance + distance);
    const float delta = mCameraBoomDistance - boomDistance;
    const glm::vec3 forward = mRotation * glm::vec3(0.f, 0.f, 1.f);
    mPosition += forward * delta;
}

void EditorCamera::moveFirstPerson()
{
    const auto timeStep = timers::deltaTime<float>();
    
    const glm::vec2 mouseOffset = engine::eventHandler->getMouseDelta();
    
    mPanAngles -= glm::radians(mouseOffset) * mMouseSpeed;
    
    mRotation = glm::angleAxis(static_cast<float>(mPanAngles.x), glm::vec3(0.f, 1.f, 0.f))
                * glm::angleAxis(static_cast<float>(mPanAngles.y), glm::vec3(1.f, 0.f, 0.f));
    
    
    mPosition.y += mSpeed * timeStep * mInputDirection.y;
    mInputDirection.y = 0.f;
    if (glm::length(mInputDirection) > 0.f)
        mInputDirection = glm::normalize(mInputDirection);
    
    mPosition += mRotation * (mSpeed * timeStep * mInputDirection);
}

const glm::mat4 &EditorCamera::getViewMatrix() const
{
    return mViewMatrix;
}

void EditorCamera::onDrawUi()
{
    PROFILE_FUNC();
    if (!showCameraSettings)
        return;

    ImGui::Begin("Renderer Settings", &showCameraSettings);
    drawRendererSettings();

    if (ImGui::CollapsingHeader("Camera Details"))
    {
        if (ImGui::TreeNode("Controls"))
        {
            ImGui::Text("Move  - WASD");
            ImGui::Text("Fly Down - Q");
            ImGui::Text("Fly Up - E");
            ImGui::Text("Look  - Right Mouse");
            
            ImGui::TreePop();
        }
        if (ImGui::TreeNode("Transform"))
        {
            ImGui::DragFloat3("Positon", glm::value_ptr(mPosition));
            ImGui::DragFloat4("Rotation", glm::value_ptr(mRotation));

            ImGui::TreePop();
        }
        float fovYDegrees = glm::degrees(mFovY);
        ImGui::SliderFloat("FOV Y", &fovYDegrees, 10, 180);
        mFovY = glm::radians(fovYDegrees);
        ImGui::DragFloat("EV100", &mEV100, 0.01f);
    }
    
    if (ImGui::CollapsingHeader("Post-processing Settings"))
    {
        for (std::unique_ptr<PostProcessLayer> &postProcessLayer : mPostProcessStack)
            ui::draw(postProcessLayer);
    }

    ImGui::End();
}

glm::mat4 EditorCamera::getProjectionMatrix() const
{
    return mProjectionMatrix;
}

CameraSettings EditorCamera::toSettings()
{
    return { mFovY, mNearClip, mFarClip, mViewMatrix, mEV100, mPostProcessStack };
}

void EditorCamera::rotateThirdPerson()
{
    const glm::vec2 mouseOffset = engine::eventHandler->getMouseDelta();
    
    const glm::vec3 anchorDirection = mRotation * glm::vec3(0.f, 0.f, 1.f);
    
    // We never want to roll the camera, so we use the "pan angles" instead.
    mPanAngles -= glm::radians(mouseOffset) * mRotationSpeed;
    mRotation = glm::angleAxis(static_cast<float>(mPanAngles.x), glm::vec3(0.f, 1.f, 0.f))
                * glm::angleAxis(static_cast<float>(mPanAngles.y), glm::vec3(1.f, 0.f, 0.f));
    
    const glm::vec3 newAnchorDirection = mRotation * glm::vec3(0.f, 0.f, 1.f);
    mPosition += mCameraBoomDistance * -(anchorDirection - newAnchorDirection);
}

void EditorCamera::gotoSelectedActor()
{
    Ref<engine::Actor> actor = engine::editor->getSelectedActor();
    if (!actor.isValid())
        return;
    
    const glm::vec3 actorPosition = actor->getWorldPosition();
    const glm::vec3 forwardDirection = mRotation * glm::vec3(0.f, 0.f, 1.f);
    mPosition = actorPosition + forwardDirection * mCameraBoomDistance;
}

glm::vec3 EditorCamera::getEndOfBoomArmPosition() const
{
    return mRotation * glm::vec3(0.f, 0.f, 1.f) * -mCameraBoomDistance + mPosition;
}



