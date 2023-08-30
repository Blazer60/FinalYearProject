/**
 * @file MainCamera.cpp
 * @author Ryan Purse
 * @date 13/02/2022
 */


#include "MainCamera.h"
#include "Timers.h"
#include "imgui.h"
#include "WindowHelpers.h"
#include "BloomPass.h"
#include "ColourGrading.h"
#include "Editor.h"
#include "EngineState.h"
#include "EventHandler.h"

MainCamera::MainCamera()
{
    init();
}

MainCamera::MainCamera(const glm::vec3 &position)
    : mPosition(position)
{
    init();
}

void MainCamera::init()
{
    const glm::vec2 size = glm::ivec2(window::bufferSize());
    mProjectionMatrix = glm::perspective(mFovY, size.x / size.y, mNearClip, mFarClip);
    
    mPostProcessStack.emplace_back(std::make_unique<BloomPass>());
    mPostProcessStack.emplace_back(std::make_unique<ColourGrading>());
    
    mFocusActorEventToken = engine::eventHandler->viewport.onFocusActor.subscribe([this]() { gotoSelectedActor(); });
    mOrbitEventToken = engine::eventHandler->viewport.thirdPerson.onOrbitCamera.subscribe([this](float value) { rotateThirdPerson(); });
    mDoMoveToken = engine::eventHandler->viewport.firstPerson.onStateChanged.subscribe([this](bool isActive) { mDoMoveAction = isActive; });
    mMoveForwardEventToken = engine::eventHandler->viewport.firstPerson.onMoveForward.subscribe([this](float value) { mInputDirection.z = value; });
    mMoveRightEventToken = engine::eventHandler->viewport.firstPerson.onMoveRight.subscribe([this](float value) { mInputDirection.x = value; });
    mMoveUpEventToken = engine::eventHandler->viewport.firstPerson.onMoveUp.subscribe([this](float value) { mInputDirection.y = value; });
    mZoomViewportToken = engine::eventHandler->viewport.onZoom.subscribe([this](float zoomValue){ zoomCamera(-zoomValue * mCameraBoomDelta); });
    mZoomThirdPersonToken = engine::eventHandler->viewport.thirdPerson.onZoomCamera.subscribe([this](float zoomValue){ zoomCamera(engine::eventHandler->getMouseDelta().y); });
}

MainCamera::~MainCamera()
{
    engine::eventHandler->viewport.onFocusActor.unSubscribe(mFocusActorEventToken);
    engine::eventHandler->viewport.thirdPerson.onOrbitCamera.unSubscribe(mOrbitEventToken);
    engine::eventHandler->viewport.firstPerson.onMoveForward.unSubscribe(mMoveForwardEventToken);
    engine::eventHandler->viewport.firstPerson.onMoveRight.unSubscribe(mMoveRightEventToken);
    engine::eventHandler->viewport.firstPerson.onMoveUp.unSubscribe(mMoveUpEventToken);
    engine::eventHandler->viewport.firstPerson.onStateChanged.unSubscribe(mDoMoveToken);
    engine::eventHandler->viewport.onZoom.unSubscribe(mZoomViewportToken);
    engine::eventHandler->viewport.thirdPerson.onZoomCamera.unSubscribe(mZoomThirdPersonToken);
}

void MainCamera::update()
{
    move();
    mProjectionMatrix = glm::perspective(mFovY, window::aspectRatio(), mNearClip, mFarClip);
    
    mViewMatrix = glm::translate(glm::mat4(1.f), mPosition)
                * glm::mat4(mRotation);
    mViewMatrix = glm::inverse(mViewMatrix);
    
    mVpMatrix   = mProjectionMatrix * mViewMatrix;
}

void MainCamera::move()
{
    if (mDoMoveAction)
        moveFirstPerson();
    
    mInputDirection = glm::vec3(0.f);
}

void MainCamera::zoomCamera(float distance)
{
    const float boomDistance = mCameraBoomDistance;
    mCameraBoomDistance = glm::max(mCameraBoomMin, mCameraBoomDistance + distance);
    const float delta = mCameraBoomDistance - boomDistance;
    const glm::vec3 forward = mRotation * glm::vec3(0.f, 0.f, 1.f);
    mPosition += forward * delta;
}

void MainCamera::moveFirstPerson()
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

const glm::mat4 &MainCamera::getVpMatrix() const
{
    return mVpMatrix;
}

void MainCamera::setProjectionMatrix(glm::vec2 viewSize)
{
    mProjectionMatrix = glm::perspective(mFovY, viewSize.x / viewSize.y, mNearClip, mFarClip);
}

const glm::mat4 &MainCamera::getViewMatrix() const
{
    return mViewMatrix;
}

void MainCamera::onDrawUi()
{
    if (ImGui::CollapsingHeader("Camera Details"))
    {
        if (ImGui::TreeNode("Controls"))
        {
            ImGui::Text("Move  - WASD");
            ImGui::Text("Ctrl  - Fly Down");
            ImGui::Text("Space - Fly Up");
            ImGui::Text("Look  - Mouse");
            
            ImGui::TreePop();
        }
        if (ImGui::TreeNode("Transform"))
        {
            ImGui::Text("Pos: %f, %f, %f", mPosition.x, mPosition.y, mPosition.z);
            ImGui::Text("Rot: %f, %f, %f, %f,", mRotation.w, mRotation.x, mRotation.y, mRotation.z);
            
            ImGui::TreePop();
        }
        float fovYDegrees = glm::degrees(mFovY);
        ImGui::SliderFloat("FOV Y", &fovYDegrees, 10, 180);
        mFovY = glm::radians(fovYDegrees);
    }
    
    if (ImGui::CollapsingHeader("Post-processing Settings"))
    {
        for (std::unique_ptr<PostProcessLayer> &postProcessLayer : mPostProcessStack)
            ui::draw(postProcessLayer);
    }
    
}

const glm::vec3 &MainCamera::getPosition() const
{
    return mPosition;
}

const glm::quat &MainCamera::getRotation() const
{
    return mRotation;
}

glm::mat4 MainCamera::getProjectionMatrix() const
{
    return mProjectionMatrix;
}

CameraSettings MainCamera::toSettings()
{
    return { mFovY, mNearClip, mFarClip, mViewMatrix, mPostProcessStack };
}

void MainCamera::rotateThirdPerson()
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

void MainCamera::gotoSelectedActor()
{
    Ref<engine::Actor> actor = engine::editor->getSelectedActor();
    if (!actor.isValid())
        return;
    
    const glm::vec3 actorPosition = actor->getWorldPosition();
    const glm::vec3 forwardDirection = mRotation * glm::vec3(0.f, 0.f, 1.f);
    mPosition = actorPosition + forwardDirection * mCameraBoomDistance;
}

glm::vec3 MainCamera::getEndOfBoomArmPosition() const
{
    return mRotation * glm::vec3(0.f, 0.f, 1.f) * -mCameraBoomDistance + mPosition;
}



