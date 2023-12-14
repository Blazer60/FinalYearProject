/**
 * @file Viewport.cpp
 * @author Ryan Purse
 * @date 08/08/2023
 */


#include "Viewport.h"
#include "imgui.h"
#include "WindowHelpers.h"
#include "GraphicsState.h"
#include "Core.h"
#include "Editor.h"
#include "ImGuizmo.h"
#include "gtc/type_ptr.hpp"
#include "gtx/matrix_decompose.hpp"
#include "EngineMath.h"
#include "ProfileTimer.h"
#include "EngineState.h"
#include "AssimpLoader.h"
#include "Scene.h"
#include "MeshRenderer.h"
#include "RigidBody.h"
#include "SceneLoader.h"
#include "Ui.h"

namespace engine
{
    Viewport::Viewport()
        : mEditorCamera(glm::vec3(0.f, 5.f, 0.f))
    {

    }

    void Viewport::init()
    {
        mViewportWindow = glfwGetCurrentContext();
        mTranslateGizmoToken = eventHandler->editor.viewport.onGizmoTranslate.subscribe([this]() { mOperation = ImGuizmo::OPERATION::TRANSLATE; });
        mRotateGizmoToken = eventHandler->editor.viewport.onGizmoRotate.subscribe([this]() { mOperation = ImGuizmo::OPERATION::ROTATE; });
        mScaleGizmoToken = eventHandler->editor.viewport.onGizmoScale.subscribe([this]() { mOperation = ImGuizmo::OPERATION::SCALE; });
        mFirstPersonToken = eventHandler->editor.viewport.firstPerson.onStateChanged.subscribe([this](bool state) { toggleMouseState(state); });
        mThirdPersonToken = eventHandler->editor.viewport.thirdPerson.onStateChanged.subscribe([this](bool state) { toggleMouseState(state); });
        
        mViewportImages = {
            ViewportImage { "Default",      []() -> const TextureBufferObject& { return graphics::renderer->getPrimaryBuffer(); } },
            ViewportImage { "Position",     []() -> const TextureBufferObject& { return graphics::renderer->getPositionBuffer(); } },
            ViewportImage { "Normal",       []() -> const TextureBufferObject& { return graphics::renderer->getNormalBuffer(); } },
            ViewportImage { "Albedo",       []() -> const TextureBufferObject& { return graphics::renderer->getAlbedoBuffer(); } },
            ViewportImage { "Emissive",     []() -> const TextureBufferObject& { return graphics::renderer->getEmissiveBuffer(); } },
            ViewportImage { "Lighting",     []() -> const TextureBufferObject& { return graphics::renderer->getLightBuffer(); } },
            ViewportImage { "Depth",        []() -> const TextureBufferObject& { return graphics::renderer->getDepthBuffer(); } },
            ViewportImage { "Roughness",    []() -> const TextureBufferObject& { return graphics::renderer->getRoughnessBuffer(); } },
            ViewportImage { "Metallic",     []() -> const TextureBufferObject& { return graphics::renderer->getMetallicBuffer(); } },
            ViewportImage { "SSR",          []() -> const TextureBufferObject& { return graphics::renderer->getSsrBuffer(); } },
            ViewportImage { "Reflections",  []() -> const TextureBufferObject& { return graphics::renderer->getReflectionBuffer(); } },
            ViewportImage { "Debug View",   []() -> const TextureBufferObject& { return graphics::renderer->getDebugBuffer(); } },
        };
    }

    Viewport::~Viewport()
    {
        eventHandler->editor.viewport.onGizmoTranslate.unSubscribe(mTranslateGizmoToken);
        eventHandler->editor.viewport.onGizmoRotate.unSubscribe(mRotateGizmoToken);
        eventHandler->editor.viewport.onGizmoScale.unSubscribe(mScaleGizmoToken);
        eventHandler->editor.viewport.firstPerson.onStateChanged.unSubscribe(mFirstPersonToken);
        eventHandler->editor.viewport.thirdPerson.onStateChanged.unSubscribe(mThirdPersonToken);
    }
    
    glm::vec2 Viewport::getSize() const
    {
        return mSize;
    }

    void Viewport::drawTopBar()
    {
        ImGui::SetCursorPosX(ImGui::GetCursorPosX() + (ImGui::GetColumnWidth() / 2) - 12.f);
        if (!core->isInPlayMode())
        {
            if (ui::imageButton("PlayButton", mPlayButton.id(), glm::ivec2(24)))
            {
                core->beginPlay();
                if (!mIsSimulating)
                    beginPlay();
            }
        }
        else
        {
            if (ui::imageButton("StopButton", mStopButton.id(), glm::ivec2(24)))
            {
                core->endPlay();
                mPlayModeCamera.nullify();
            }
        }
        ImGui::SameLine();
        ImGui::Checkbox("Simulate", &mIsSimulating);

        if (ImGui::RadioButton("Move", mOperation == ImGuizmo::OPERATION::TRANSLATE))
            mOperation = ImGuizmo::OPERATION::TRANSLATE;
        ImGui::SameLine();
        if (ImGui::RadioButton("Rotate", mOperation == ImGuizmo::OPERATION::ROTATE))
            mOperation = ImGuizmo::OPERATION::ROTATE;
        ImGui::SameLine();
        if (ImGui::RadioButton("Scale", mOperation == ImGuizmo::OPERATION::SCALE))
            mOperation = ImGuizmo::OPERATION::SCALE;

        ImGui::SameLine(0.f, ImGui::GetStyle().ItemSpacing.x * 3.f);
        if (ImGui::RadioButton("Local", mMode == ImGuizmo::MODE::LOCAL))
            mMode = ImGuizmo::MODE::LOCAL;
        ImGui::SameLine();
        if (ImGui::RadioButton("World", mMode == ImGuizmo::MODE::WORLD))
            mMode = ImGuizmo::MODE::WORLD;
        ImGui::SameLine();
        ImGui::Checkbox("Force 1080p", &mForce1080p);
        ImGui::SameLine();
        ImGui::Checkbox("Debug Overlay", &mShowDebugOverlay);

        ImGui::SameLine();
        ImGui::SetCursorPosX(ImGui::GetCursorPosX() + ImGui::GetColumnWidth() - 100.f);
        ImGui::SetNextItemWidth(100.f);
        if (ImGui::BeginCombo("##ViewImage", mViewportImages[mCurrentSelectedImage].name.c_str()))
        {
            for (int i = 0; i < mViewportImages.size(); ++i)
            {
                const bool isSelected = (mCurrentSelectedImage == i);
                if (ImGui::Selectable(mViewportImages[i].name.c_str(), isSelected))
                    mCurrentSelectedImage = i;

                if (isSelected)
                    ImGui::SetItemDefaultFocus();
            }
            ImGui::EndCombo();
        }
    }

    void Viewport::drawEditorView()
    {
        ImGui::BeginChild("ImageWindow");

        const ImVec2 cursorPos = ImGui::GetCursorPos();
        const ImVec2 regionSize = mForce1080p ? ImVec2(1920.f, 1080.f) : ImGui::GetContentRegionAvail();
        window::setBufferSize(glm::ivec2(regionSize.x, regionSize.y));
        const TextureBufferObject &texture = mViewportImages[mCurrentSelectedImage].requestTexture();
        ImGui::Image(reinterpret_cast<void *>(texture.getId()), regionSize, ImVec2(0, 1), ImVec2(1, 0));
        if (ImGui::BeginDragDropTarget())
        {
            if (const ImGuiPayload *payload0 = ImGui::AcceptDragDropPayload(resourceModelPayload))
                editor->createModel(*reinterpret_cast<std::filesystem::path*>(payload0->Data));
            else if (const ImGuiPayload *payload1 = ImGui::AcceptDragDropPayload(resourceScenePayload))
            {
                if (const auto path = *reinterpret_cast<std::filesystem::path*>(payload1->Data); !path.empty())
                    core->setScene(load::scene(path), path);
            }
        }

        if (mShowDebugOverlay)
        {
            ImGui::SetCursorPos(cursorPos);
            const TextureBufferObject &debugTexture = graphics::renderer->getDebugBuffer();
            ImGui::Image(reinterpret_cast<void *>(debugTexture.getId()), regionSize, ImVec2(0, 1), ImVec2(1, 0));
        }

        mIsHovered = ImGui::IsWindowHovered();

        if (auto *x = reinterpret_cast<GLFWwindow*>(ImGui::GetWindowViewport()->PlatformHandle); x != nullptr)
            mViewportWindow = x;

        Ref<Actor> selectedActor = editor->getSelectedActor();
        if (selectedActor.isValid())
        {
            const float windowWidth = ImGui::GetWindowWidth();
            const float windowHeight = ImGui::GetWindowHeight();
            ImGuizmo::SetDrawlist();
            ImGuizmo::SetRect(ImGui::GetWindowPos().x, ImGui::GetWindowPos().y, windowWidth, windowHeight);
            ImGuizmo::SetOrthographic(false);
            ImGuizmo::Enable(true);

            glm::mat4 actorTransform = selectedActor->getTransform();

            glm::mat4 view = mEditorCamera.getViewMatrix();
            glm::mat4 projection = mEditorCamera.getProjectionMatrix();

            if (ImGuizmo::Manipulate(glm::value_ptr(view), glm::value_ptr(projection),
                                     mOperation, mMode, glm::value_ptr(actorTransform)))
            {
                if (Actor *parent = selectedActor->getParent(); parent != nullptr)
                {
                    // Undo any parent transforms before applying a local transform.
                    const glm::mat4 parentTransform = parent->getTransform();
                    actorTransform = glm::inverse(parentTransform) * actorTransform;
                }
                math::decompose(actorTransform, selectedActor->position, selectedActor->rotation, selectedActor->scale);
                if (Ref<RigidBody> rigidBody = selectedActor->getComponent<RigidBody>(false); rigidBody.isValid())
                {
                    rigidBody->alignWithActorTransform();
                }
            }
        }

        ImGui::EndChild();
    }

    void Viewport::drawPlayModeView() const
    {
        ImGui::BeginChild("ImageWindow");

        const ImVec2 regionSize = mForce1080p ? ImVec2(1920.f, 1080.f) : ImGui::GetContentRegionAvail();
        window::setBufferSize(glm::ivec2(regionSize.x, regionSize.y));

        const TextureBufferObject &texture = graphics::renderer->getPrimaryBuffer();
        ImGui::Image(reinterpret_cast<void *>(texture.getId()), regionSize, ImVec2(0, 1), ImVec2(1, 0));

        ImGui::EndChild();
    }

    void Viewport::onDrawUi()
    {
        PROFILE_FUNC();
        if (!isShowing)
            return;

        ImGui::PushID("Viewport");
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.f, 0.f));
        std::string sceneName = core->getSceneName();
        if (sceneName.empty())
            sceneName = "Unsaved";

        const std::string windowName = sceneName + "###ViewportWindow";
        ImGui::Begin(windowName.c_str(), &isShowing);

        drawTopBar();
        if (isUsingPlayModeCamera())
            drawPlayModeView();
        else
            drawEditorView();

        ImGui::End();
        ImGui::PopStyleVar();
        ImGui::PopID();
    }
    
    bool Viewport::isHovered() const
    {
        return mIsHovered;
    }

    bool Viewport::isUsingPlayModeCamera() const
    {
        return core->isInPlayMode() && mPlayModeCamera.isValid();
    }

    void Viewport::toggleMouseState(const bool newState) const
    {
        if (newState)
            glfwSetInputMode(mViewportWindow, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
        else
            glfwSetInputMode(mViewportWindow, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
    }
    
    GLFWwindow *Viewport::getViewportContext() const
    {
        return mViewportWindow;
    }

    bool Viewport::isDebugViewOn() const
    {
        return mShowDebugOverlay || mViewportImages[mCurrentSelectedImage].name == "Debug View";
    }

    void Viewport::preRender()
    {
        if (isUsingPlayModeCamera())
            graphics::renderer->submit(mPlayModeCamera->toCameraSettings());
        else
            graphics::renderer->submit(mEditorCamera.toSettings());
    }

    void Viewport::update()
    {
        mEditorCamera.update();
    }

    void Viewport::beginPlay()
    {
        mPlayModeCamera.nullify();
        const auto cameras = core->getScene()->findComponents<Camera>();
        for (Ref<Camera> camera : cameras)
        {
            if (camera->isMainCamera())
            {
                mPlayModeCamera = camera;
                return;
            }
        }

        if (!mPlayModeCamera.isValid() && !cameras.empty())
            mPlayModeCamera = cameras[0];  // Falback is a main camera isn't set.
    }

    EditorCamera* Viewport::getCamera()
    {
        return &mEditorCamera;
    }
}
