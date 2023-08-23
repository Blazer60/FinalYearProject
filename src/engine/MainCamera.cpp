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
    
    mKeyboardToken = engine::editor->onKeyPressed.subscribe([this](const ImGuiKey &key, bool isPressed) {
        if (key == ImGuiKey_LeftAlt)
            mEnableThirdPerson = isPressed;
        
        if (key == ImGuiKey_F)
            gotoSelectedActor();
    });
    
    mMouseToken = engine::editor->onMouseClicked.subscribe([this](ImGuiMouseButton button, bool isClicked) {
        if (button == ImGuiMouseButton_Right)
            mIsRightMousePressed = isClicked;
        if (button == ImGuiMouseButton_Left)
            mIsLeftMousePressed = isClicked;
    });
}

MainCamera::~MainCamera()
{
    engine::editor->onKeyPressed.unSubscribe(mKeyboardToken);
    engine::editor->onMouseClicked.unSubscribe(mMouseToken);
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
    if (!engine::editor->isViewportHovered())
        return;
    
    const float boomDistance = mCameraBoomDistance;
    mCameraBoomDistance = glm::max(mCameraBoomMin, mCameraBoomDistance + -engine::editor->getMouseWheel() * mCameraBoomDelta);
    const float delta = mCameraBoomDistance - boomDistance;
    const glm::vec3 forward = mRotation * glm::vec3(0.f, 0.f, 1.f);
    mPosition += forward * delta;
    
    if (mEnableThirdPerson)
    {
        if (mIsLeftMousePressed)
            rotateThirdPerson();
    }
    else
    {
        if (mIsRightMousePressed)
            moveFirstPerson();
    }
    
    
    mInputDirection = glm::vec3(0.f);
}

void MainCamera::moveFirstPerson()
{
    const auto timeStep = timers::deltaTime<float>();
    
    if (ImGui::IsKeyDown(ImGuiKey_W))
        mInputDirection.z -= 1.f;
    if (ImGui::IsKeyDown(ImGuiKey_S))
        mInputDirection.z += 1.f;
    if (ImGui::IsKeyDown(ImGuiKey_A))
        mInputDirection.x -= 1.f;
    if (ImGui::IsKeyDown(ImGuiKey_D))
        mInputDirection.x += 1.f;
    if (ImGui::IsKeyDown(ImGuiKey_Q))
        mInputDirection.y -= 1.f;
    if (ImGui::IsKeyDown(ImGuiKey_E))
        mInputDirection.y += 1.f;
    
    ImGuiIO &io = ImGui::GetIO();
    glm::dvec2 mouseOffset { io.MouseDelta.x, io.MouseDelta.y };
    
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
    ImGuiIO &io = ImGui::GetIO();
    const glm::vec2 mouseOffset { io.MouseDelta.x, io.MouseDelta.y };
    
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
    if (!engine::editor->isViewportHovered())
        return;
    
    engine::Actor *actor = engine::editor->getSelectedActor();
    if (!actor)
        return;
    
    const glm::vec3 actorPosition = actor->position;
    const glm::vec3 forwardDirection = mRotation * glm::vec3(0.f, 0.f, 1.f);
    mPosition = actorPosition + forwardDirection * mCameraBoomDistance;
}



