/**
 * @file Actor.cpp
 * @author Ryan Purse
 * @date 07/08/2023
 */


#include "Actor.h"
#include "gtc/type_ptr.hpp"

namespace engine
{
    void Actor::OnUpdate()
    {
    
    }
    
    std::string_view Actor::getName() const
    {
        return mName;
    }
    
    void Actor::onDrawUi()
    {
        ImGui::PushID("ActorSettings");
        if (ImGui::CollapsingHeader("Transform", ImGuiTreeNodeFlags_DefaultOpen))
        {
            ImGui::DragFloat3("Position", glm::value_ptr(mPosition));
            ImGui::DragFloat4("Rotation", glm::value_ptr(mRotation));
            ImGui::DragFloat3("Scale", glm::value_ptr(mScale));
        }
        ImGui::PopID();
    }
}

