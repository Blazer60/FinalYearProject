/**
 * @file Actor.cpp
 * @author Ryan Purse
 * @date 07/08/2023
 */


#include "Actor.h"

#include <utility>
#include "gtc/type_ptr.hpp"
#include "Ui.h"
#include "Scene.h"

namespace engine
{
    Actor::Actor(std::string name)
        : mName(std::move(name))
    {
    
    }
    
    void Actor::OnUpdate()
    {
        updateTransform();
    }
    
    std::string_view Actor::getName() const
    {
        return mName;
    }
    
    void Actor::onDrawUi()
    {
        ImGui::PushID("ActorSettings");
        ImGui::SetNextItemWidth(glm::min(
            200.f,
            ImGui::GetContentRegionAvail().x - ImGui::CalcTextSize("Delete Actor").x - ImGui::GetStyle().WindowPadding.x * 2.f));
        ui::inputText("##Name", &mName);
        ui::drawToolTip("Actor's Name");
        ImGui::SameLine(ImGui::GetContentRegionAvail().x - ImGui::CalcTextSize("Delete Actor").x);
        if (ImGui::Button("Delete Actor"))
        {
            MESSAGE("Deleting Actor");
            markForDeath();
        }
        if (ImGui::CollapsingHeader("Transform", ImGuiTreeNodeFlags_DefaultOpen))
        {
            bool changedFlag = false;
            changedFlag |= ImGui::DragFloat3("Position", glm::value_ptr(position));
            changedFlag |= ImGui::DragFloat4("Rotation", glm::value_ptr(rotation));
            changedFlag |= ImGui::DragFloat3("Scale", glm::value_ptr(scale));
            
            if (changedFlag)
                updateTransform();
        }
        ImGui::PopID();
    }
    
    void Actor::updateTransform()
    {
        mTransform = glm::translate(glm::mat4(1.f), position) *  glm::mat4_cast(rotation) * glm::scale(glm::mat4(1.f), scale);
    }
    
    std::vector<std::unique_ptr<Component>> &Actor::getComponents()
    {
        return mComponents;
    }
    
    void Actor::addComponent(std::unique_ptr<Component> component)
    {
        component->attachToActor(this);
        mComponents.push_back(std::move(component));
    }
    
    glm::mat4 Actor::getTransform() const
    {
        return mTransform;
    }
    
    void Actor::markForDeath()
    {
        mScene->destroy(this);
    }
}

