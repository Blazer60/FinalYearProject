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
    
    void Actor::begin()
    {
        onBegin();
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
        if (mParent)
            mParent->removeChildActor(this);
        else
            mScene->destroy(this);
    }
    
    void Actor::update()
    {
        onUpdate();
        updateComponents();
        
        for (int i = 0; i < mActorsToAdd.size(); ++i)
        {
            mActorsToAdd[i]->begin();
            mChildren.push_back(std::move(mActorsToAdd[i]));
        }
        mActorsToAdd.clear();
        
        for (auto &child : mChildren)
            child->update();
        
        auto currentActorDestroyBuffer = mActorsToDestroy;
        if (mActorsToDestroy == &mActorDestroyBuffer0)
            mActorsToDestroy = &mActorDestroyBuffer1;
        else
            mActorsToDestroy = &mActorDestroyBuffer0;
        
        // Removing children that are marked for deletion.
        for (const Actor *actor : *currentActorDestroyBuffer)
        {
            const auto it = std::find_if(mChildren.begin(), mChildren.end(), [&actor](const Resource<Actor> &left) {
                return left.get() == actor;
            });
            
            mChildren.erase(it);
        }
        currentActorDestroyBuffer->clear();
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
        
        for (auto &component : mComponents)
            component->update();
        
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
    
    void Actor::removeChildActor(const Actor* actor)
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
                mChildren.begin(), mChildren.end(), [&actor](const Resource<Actor> &left) {
                    return left.get() == actor;
                }
            );
            
            if (it == mChildren.end())
            {
                const auto it2 = std::find_if(
                    mActorsToAdd.begin(), mActorsToAdd.end(), [&actor](const Resource<Actor> &left) {
                        return left.get() == actor;
                    }
                );
                
                if (it2 == mActorsToAdd.end())
                {
                    ERROR("Actor has this parent but is not in the child list.");
                    return;
                }
            }
            
            mActorsToDestroy->emplace(actor);
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
        auto moveOut = [](Resource<Actor> &&out) {
            // We reset the transform to world space since we aren't connect to anything.
            out->mTransform = out->getTransform();
            math::decompose(out->mTransform, out->position, out->rotation, out->scale);
            out->mParent = nullptr;
            return out;
        };
        
        const auto it = std::find_if(mChildren.begin(), mChildren.end(), [&actor](const Resource<Actor> &child) {
            return child.get() == actor;
        });
        
        if (it == mChildren.end())
        {
            const auto it2 = std::find_if(mActorsToAdd.begin(), mActorsToAdd.end(), [&actor](const Resource<Actor> &child) {
                return child.get() == actor;
            });
            
            if (it2 != mActorsToAdd.end())
            {
                Resource<Actor> out = std::move(*it2);
                mActorsToAdd.erase(it2);
                return moveOut(std::move(out));
            }
            
            LOG_MAJOR("Failed to find Actor while popping");
            return Resource<Actor>();
        }
        
        Resource<Actor> out = std::move(*it);
        mChildren.erase(it);
        
        return moveOut(std::move(out));
    }
    
    Scene *Actor::getScene()
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
}

