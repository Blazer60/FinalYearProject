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

namespace engine
{
    void Viewport::init()
    {
        mKeyboardEventToken = editor->onKeyPressed.subscribe([this](const ImGuiKey &key, bool isClicked) {
            if (!mIsHovered)
                return;
            
            if (mIsMouseDown)
                return;
            
            if (!isClicked)
                return;
            
            switch (key)
            {
                case ImGuiKey_W:
                    mOperation = ImGuizmo::OPERATION::TRANSLATE;
                    break;
                case ImGuiKey_E:
                    mOperation = ImGuizmo::OPERATION::ROTATE;
                    break;
                case ImGuiKey_R:
                    mOperation = ImGuizmo::OPERATION::SCALE;
                    break;
                default:
                    break;
            }
        });
        
        mRightMouseEventToken = editor->onMouseClicked.subscribe([this](ImGuiMouseButton button, bool isClicked) {
            if (button != ImGuiMouseButton_Right)
                return;
            
            mIsMouseDown = isClicked && mIsHovered;
            
            if (!isClicked && mIsHovered)
                glfwSetCursorPos(glfwGetCurrentContext(), mLastMousePosition.x, mLastMousePosition.y);
            
            glfwGetCursorPos(glfwGetCurrentContext(), &mLastMousePosition.x, &mLastMousePosition.y);
        });
    }
    
    Viewport::~Viewport()
    {
        editor->onKeyPressed.unSubscribe(mKeyboardEventToken);
        editor->onMouseClicked.unSubscribe(mRightMouseEventToken);
    }
    
    glm::vec2 Viewport::getSize() const
    {
        return mSize;
    }
    
    void Viewport::onDrawUi()
    {
        if (mIsMouseDown)
            ImGui::SetMouseCursor(ImGuiMouseCursor_None);
        
        ImGui::PushID("Viewport");
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.f, 0.f));
        ImGui::Begin("Window");
        
        ImVec2 regionSize = ImGui::GetContentRegionAvail();
        window::setBufferSize(glm::ivec2(regionSize.x, regionSize.y));
        
        const TextureBufferObject &texture = graphics::renderer->getPrimaryBuffer();
        ImVec2 origin = ImGui::GetCursorPos();
        ImGui::Image(reinterpret_cast<void *>(texture.getId()), regionSize, ImVec2(0, 1), ImVec2(1, 0));
        ImGui::SetCursorPos(origin);
        if (ImGui::RadioButton("Move", mOperation == ImGuizmo::OPERATION::TRANSLATE))
            mOperation = ImGuizmo::OPERATION::TRANSLATE;
        ImGui::SameLine();
        if (ImGui::RadioButton("Rotate", mOperation == ImGuizmo::OPERATION::ROTATE))
            mOperation = ImGuizmo::OPERATION::ROTATE;
        ImGui::SameLine();
        if (ImGui::RadioButton("Scale", mOperation == ImGuizmo::OPERATION::SCALE))
            mOperation = ImGuizmo::OPERATION::SCALE;
        
        mIsHovered = ImGui::IsWindowHovered();
        
        Actor *selectedActor = editor->getSelectedActor();
        if (selectedActor)
        {
            const float windowWidth = ImGui::GetWindowWidth();
            const float windowHeight = ImGui::GetWindowHeight();
            ImGuizmo::SetDrawlist();
            ImGuizmo::SetRect(ImGui::GetWindowPos().x, ImGui::GetWindowPos().y, windowWidth, windowHeight);
            ImGuizmo::SetOrthographic(false);
            ImGuizmo::Enable(true);
            
            glm::mat4 actorTransform = selectedActor->getTransform();
            
            MainCamera *camera = core->getCamera();
            glm::mat4 view = camera->getViewMatrix();
            glm::mat4 projection = camera->getProjectionMatrix();
            
            if (ImGuizmo::Manipulate(glm::value_ptr(view), glm::value_ptr(projection),
                                 mOperation, ImGuizmo::MODE::LOCAL, glm::value_ptr(actorTransform)))
            {
                math::decompose(actorTransform, selectedActor->position, selectedActor->rotation, selectedActor->scale);
            }
        }
        ImGui::End();
        ImGui::PopStyleVar();
        ImGui::PopID();
    }
    
    bool Viewport::isHovered() const
    {
        return mIsHovered;
    }
    
    
}
