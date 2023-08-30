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
            markForDeath();
        
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
        if (mParent != nullptr)
            return mParent->getTransform() * mTransform;
        return mTransform;
    }
    
    void Actor::markForDeath()
    {
        if (mParent)
            mParent->removeChildActor(this);
        else
            mScene->destroy(this);
    }
    
    void Actor::update()
    {
        onUpdate();
        
        for (auto &component : getComponents())
            component->update();
        
        for (Resource<Actor> &child : mChildren)
            child->update();
        
        // Removing components that are marked for deletion.
        for (const uint32_t index : mComponentsToDestroy)
            mComponents.erase(mComponents.begin() + index);
        
        mComponentsToDestroy.clear();
        
        // Removing children that are marked for deletion.
        for (const uint32_t index : mActorsToDestroy)
            mChildren.erase(mChildren.begin() + index);
        
        mActorsToDestroy.clear();
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
        
        mComponentsToDestroy.emplace(index);
    }
    
    
    void Actor::removeChildActor(Actor* actor)
    {
        if (actor == nullptr)
        {
            WARN("This actor cannot be removed since it is nullptr!");
            return;
        }
        
        // Breadth-first-search as this will stop recursive calls sooner.
        if (actor->mParent == this)
        {
            const auto it = std::find_if(
                mChildren.begin(), mChildren.end(), [&actor](const Resource<Actor> &left) {
                    return left.get() == actor;
                }
            );
            
            if (it == mChildren.end())
                return;
            
            const uint32_t index = std::distance(mChildren.begin(), it);
            
            mActorsToDestroy.emplace(index);
        }
        else
        {
            for (Resource<Actor> &child : mChildren)
                child->removeChildActor(actor);
        }
    }
    
    std::vector<Resource<Actor>> &Actor::getChildren()
    {
        return mChildren;
    }
    
    glm::mat4 Actor::getLocalTransform() const
    {
        return mTransform;
    }
    
    Actor *Actor::getParent()
    {
        return mParent;
    }
    
    glm::vec3 Actor::getWorldPosition()
    {
        if (mParent != nullptr)
            return mParent->getTransform() * glm::vec4(position, 1.f);
        return position;
    }
    
    Resource<Actor> Actor::popActor(Actor *actor)
    {
        const auto it = std::find_if(mChildren.begin(), mChildren.end(), [&actor](const Resource<Actor> &child) {
            return child.get() == actor;
        });
        
        if (it == mChildren.end())
        {
            LOG_MAJOR("Failed to find actor while popping");
            return Resource<Actor>();
        }
        
        Resource<Actor> out = std::move(*it);
        mChildren.erase(it);
        
        out->mParent = nullptr;
        
        return out;
    }
    
    Scene *Actor::getScene()
    {
        return mScene;
    }
}

