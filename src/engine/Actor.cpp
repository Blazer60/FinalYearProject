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
    
    void Actor::onUpdate()
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
    
    std::vector<Resource<Component>> &Actor::getComponents()
    {
        return mComponents;
    }
    
    Ref<Component> Actor::addComponent(Resource<Component> &&component)
    {
        Ref<Component> ref = component;
        component->attachToActor(this);
        mComponents.push_back(std::move(component));
        
        return ref;
    }
    
    glm::mat4 Actor::getTransform() const
    {
        return mTransform;
    }
    
    void Actor::markForDeath()
    {
        mScene->destroy(this);
    }
    
    void Actor::update()
    {
        onUpdate();
        
        // Removing components that are marked for deletion.
        for (const uint32_t index : mToDestroy)
            mComponents.erase(mComponents.begin() + index);
        
        mToDestroy.clear();
    }
    
    void Actor::removeComponent(Component *component)
    {
        if (component->getActor() != this)
        {
            WARN("This actor does not own this component. The component will not be destroyed.");
            return;
        }
        
        const auto it = std::find_if(mComponents.begin(), mComponents.end(), [&component](const Ref<Component> &left) {
            return left.get() == component;
        });
        
        if (it == mComponents.end())
        {
            WARN("Could not find the component attached to this actor.");
            return;
        }
        
        const uint32_t index = std::distance(mComponents.begin(), it);
        
        mToDestroy.emplace(index);
    }
}

