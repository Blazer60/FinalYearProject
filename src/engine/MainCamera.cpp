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
    
    mKeyboardToken = engine::editor->onIoKeyboardEvent.subscribe([this](const std::vector<ImGuiKey> &keys) {
        for (const ImGuiKey &key : keys)
        {
            switch (key)
            {
                case ImGuiKey_W:
                    mInputDirection.z -= 1.f;
                    break;
                case ImGuiKey_S:
                    mInputDirection.z += 1.f;
                    break;
                case ImGuiKey_A:
                    mInputDirection.x -= 1.f;
                    break;
                case ImGuiKey_D:
                    mInputDirection.x += 1.f;
                    break;
                case ImGuiKey_Q:
                    mInputDirection.y -= 1.f;
                    break;
                case ImGuiKey_E:
                    mInputDirection.y += 1.f;
                    break;
                default:
                    break;
            }
        }
    });
    
    mMouseToken = engine::editor->onMouseClicked.subscribe([this](ImGuiMouseButton button, bool isClicked) {
        mEnableFirstPerson = button == ImGuiMouseButton_Right && isClicked;
    });
}

MainCamera::~MainCamera()
{
    engine::editor->onIoKeyboardEvent.unSubscribe(mKeyboardToken);
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
    if (!engine::editor->isViewportFocused())
        return;
    
    if (!mEnableFirstPerson)
        return;
    
    const auto timeStep = timers::deltaTime<float>();
    
    ImGuiIO &io = ImGui::GetIO();
    glm::dvec2 mouseOffset { io.MouseDelta.x, io.MouseDelta.y };
    
    mPanAngles -= glm::radians(mouseOffset) * mMouseSpeed;
    
    const glm::vec3 up    = glm::vec3(0.f, 1.f, 0.f);
    const glm::vec3 right = glm::vec3(1.f, 0.f, 0.f);
    
    mRotation = glm::angleAxis(static_cast<float>(mPanAngles.x), up)
              * glm::angleAxis(static_cast<float>(mPanAngles.y), right);

    
    mPosition.y += mSpeed * timeStep * mInputDirection.y;
    mInputDirection.y = 0.f;
    if (glm::length(mInputDirection) > 0.f)
        mInputDirection = glm::normalize(mInputDirection);
    
    mPosition += mRotation * (mSpeed * timeStep * mInputDirection);
    
    mInputDirection = glm::vec3(0.f);
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



