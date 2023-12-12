/**
 * @file Actor.cpp
 * @author Ryan Purse
 * @date 07/08/2023
 */


#include "Actor.h"

#include <utility>

#include "Core.h"
#include "EngineState.h"
#include "gtc/type_ptr.hpp"
#include "Ui.h"
#include "Scene.h"

namespace engine
{
    Actor::Actor(std::string name)
        : mName(std::move(name))
    {
    
    }
    
    void Actor::begin()
    {
        onBegin();
    }
    
    
    void Actor::onUpdate()
    {
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

        const auto id = std::to_string(mId);
        ImGui::TextColored(ImVec4(0.3f, 0.3f, 0.3f, 1.f), id.c_str());

        if (ImGui::CollapsingHeader("Transform", ImGuiTreeNodeFlags_DefaultOpen))
        {
            bool changedFlag = false;
            changedFlag |= ImGui::DragFloat3("Position", glm::value_ptr(position));
            glm::vec3 euler = glm::degrees(glm::eulerAngles(rotation));
            changedFlag |= ImGui::DragFloat3("Rotation", glm::value_ptr(euler));
            rotation = glm::quat(glm::radians(euler));
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
    
    glm::mat4 Actor::getTransform() const
    {
        if (mParent != nullptr)
            return mParent->getTransform() * mTransform;
        return mTransform;
    }
    
    void Actor::markForDeath()
    {
        if (mParent != nullptr)
            mParent->removeChildActor(this);
        mScene->destroy(this);
    }
    
    void Actor::update()
    {
        updateTransform();
        if (core->isInPlayMode())
            onUpdate();
        updateComponents();

        if (!mChildrenToRemove.empty())
        {
            mChildren.erase(std::remove_if(mChildren.begin(), mChildren.end(), [this](const UUID id) {
                return mChildrenToRemove.count(id) > 0;
            }), mChildren.end());
            mChildrenToRemove.clear();
        }
    }

    void Actor::fixedUpdate()
    {
        for (Ref<Component> component : mComponents)
            component->fixedUpdate();
    }

    void Actor::collisionBegin(
        Actor* otherActor, Component* myComponent,
        Component* otherComponent, const HitInfo& hitInfo)
    {
        onCollisionBegin(otherActor, myComponent, otherComponent, hitInfo);

        for (Ref<Component> component : mComponents)
            component->collisionBegin(otherActor, myComponent, otherComponent, hitInfo);
    }

    void Actor::triggerBegin(Actor* otherActor, Component* myComponent, Component* otherComponent)
    {
        onTriggerBegin(otherActor, myComponent, otherComponent);

        for (Ref<Component> component : mComponents)
            component->triggerBegin(otherActor, myComponent, otherComponent);
    }

    UUID Actor::getId() const
    {
        return mId;
    }

    void Actor::updateComponents()
    {
        // Components could create more components.
        for (int i = 0; i < mComponentsToAdd.size(); ++i)
        {
            mComponentsToAdd[i]->begin();
            mComponents.push_back(std::move(mComponentsToAdd[i]));
        }
        mComponentsToAdd.clear();

        if (core->isInPlayMode())
        {
            for (auto &component : mComponents)
                component->update();
        }

        auto currentComponentDestroyBuffer = mComponentsToDestroy;
        if (mComponentsToDestroy == &mComponentDestroyBuffer0)
            mComponentsToDestroy = &mComponentDestroyBuffer1;
        else
            mComponentsToDestroy = &mComponentDestroyBuffer0;
        
        // Removing components that are marked for deletion.
        for (const Component *component : *currentComponentDestroyBuffer)
        {
            const auto it = std::find_if(
                mComponents.begin(), mComponents.end(), [&component](const Ref<Component> &left) {
                return left.get() == component;
            });
            
            // We've already removed it in a previous pass.
            if (it != mComponents.end())
                mComponents.erase(it);
        }
        
        currentComponentDestroyBuffer->clear();
    }
    
    void Actor::removeComponent(const Component *component)
    {
        if (component == nullptr)
            return;  // Most likely already removed.
        
        if (component->getActor() != this)
        {
            WARN("This serializeActor does not own this Component. The Component will not be destroyed.");
            return;
        }
        
        const auto it = std::find_if(mComponents.begin(), mComponents.end(), [&component](const Ref<Component> &left) {
            return left.get() == component;
        });
        
        if (it == mComponents.end())
        {
            const auto it2 = std::find_if(mComponentsToAdd.begin(), mComponentsToAdd.end(), [&component](const Ref<Component> &left) {
                return left.get() == component;
            });
            
            if (it2 == mComponentsToAdd.end())
            {
                WARN("Could not find the Component attached to this Actor.");
                return;
            }
            else
            {
                // Pretend it never existed.
                mComponentsToAdd.erase(it2);
            }
        }
        
        mComponentsToDestroy->emplace(component);
    }
    
    void Actor::removeChildActor(Actor* actor)
    {
        if (actor == nullptr)
        {
            WARN("This Actor cannot be removed since it is nullptr!");
            return;
        }
        
        // Breadth-first-search as this will stop recursive calls sooner.
        if (actor->mParent == this)
        {
            const auto it = std::find_if(
                mChildren.begin(), mChildren.end(), [&actor](const UUID id) {
                    return id == actor->getId();
                }
            );

            if (it == mChildren.end())
            {
                ERROR("Actor has this parent but is not in the child list.");
                return;
            }

            // We reset the transform to world space since we aren't connect to anything.
            actor->mTransform = actor->getTransform();
            math::decompose(actor->mTransform, actor->position, actor->rotation, actor->scale);
            actor->mParent = nullptr;
            mChildrenToRemove.insert(actor->getId());
        }
        else
        {
            for (const UUID childId : mChildren)
                mScene->getActor(childId)->removeChildActor(actor);
        }
    }
    
    std::vector<UUID> &Actor::getChildren()
    {
        return mChildren;
    }
    
    glm::mat4 Actor::getLocalTransform() const
    {
        return mTransform;
    }
    
    Actor *Actor::getParent() const
    {
        return mParent;
    }
    
    glm::vec3 Actor::getWorldPosition() const
    {
        if (mParent != nullptr)
            return mParent->getTransform() * glm::vec4(position, 1.f);
        return position;
    }
    
    Scene *Actor::getScene() const
    {
        return mScene;
    }
    
    void Actor::removeComponent(const Ref<Component> &component)
    {
        if (component.isValid())
            removeComponent(component.get());
    }
    
    void Actor::onBegin()
    {
    
    }

    void Actor::onCollisionBegin(
        Actor* otherActor, Component* myComponent,
        Component* otherComponent, const HitInfo& hitInfo)
    {

    }

    void Actor::onTriggerBegin(Actor* otherActor, Component* myComponent, Component* otherComponent)
    {

    }
}

