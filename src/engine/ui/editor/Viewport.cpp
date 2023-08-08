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
    glm::vec2 Viewport::getSize() const
    {
        return mSize;
    }
    
    void Viewport::onDrawUi()
    {
        ImGui::PushID("Viewport");
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.f, 0.f));
        ImGui::Begin("Window");
        
        ImVec2 regionSize = ImGui::GetContentRegionAvail();
        window::setBufferSize(glm::ivec2(regionSize.x, regionSize.y));
        
        const TextureBufferObject &texture = graphics::renderer->getPrimaryBuffer();
        ImGui::Image(reinterpret_cast<void *>(texture.getId()), regionSize, ImVec2(0, 1), ImVec2(1, 0));
        
        Actor *selectedActor = editor->getSelectedActor();
        if (selectedActor)
        {
            const float windowWidth = ImGui::GetWindowWidth();
            const float windowHeight = ImGui::GetWindowHeight();
            ImGui::GetIO();
            ImGuizmo::SetDrawlist();
            ImGuizmo::SetRect(ImGui::GetWindowPos().x, ImGui::GetWindowPos().y, windowWidth, windowHeight);
            ImGuizmo::SetOrthographic(false);
            ImGuizmo::Enable(true);
            
            glm::mat4 actorTransform = selectedActor->getTransform();
            
            MainCamera *camera = core->getCamera();
            glm::mat4 view = camera->getViewMatrix();
            glm::mat4 projection = camera->getProjectionMatrix();
            
            if (ImGuizmo::Manipulate(glm::value_ptr(view), glm::value_ptr(projection),
                                 ImGuizmo::OPERATION::TRANSLATE, ImGuizmo::MODE::LOCAL, glm::value_ptr(actorTransform)))
            {
                math::decompose(actorTransform, selectedActor->position, selectedActor->rotation, selectedActor->scale);
            }
        }
        ImGui::End();
        ImGui::PopStyleVar();
        ImGui::PopID();
    }
}
