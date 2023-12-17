/**
 * @file Component.cpp
 * @author Ryan Purse
 * @date 07/08/2023
 */


#include "Component.h"
#include "Actor.h"
#include <Statistics.h>

namespace engine
{
    void Component::awake()
    {
        try
        {
            onAwake();
        }
        catch (InvalidReference &e)
        {
            ERROR("OnAwake() error in on of %'s componenets: %", mActor->getName(), e.what());
        }
    }

    void Component::begin()
    {
        try
        {
            onBegin();
        }
        catch (InvalidReference &e)
        {
            ERROR("OnBegin() error in one of %'s components: %", mActor->getName(), e.what());
        }
    }
    
    void Component::update()
    {
        onUpdate();
    }

    void Component::fixedUpdate()
    {
        onFixedUpdate();
    }

    void Component::onUpdate()
    {
    
    }

    void Component::onFixedUpdate()
    {
    }

    void Component::onDrawUi()
    {
    }
    
    glm::mat4 Component::getWorldTransform() const
    {
        if (mActor)
            return mActor->getTransform();
        WARN("The component has not been attached to an actor yet.");
        return glm::mat4(1.f);
    }
    
    void Component::attachToActor(Actor *actor)
    {
        mActor = actor;
    }

    void Component::collisionBegin(
        Actor* otherActor, Component* myComponent, Component* otherComponent, const HitInfo& hitInfo)
    {
        try
        {
            onCollisionBegin(otherActor, myComponent, otherComponent, hitInfo);
        }
        catch (InvalidReference &e)
        {
            ERROR("OnCollisionBegin() error in one of %'s components: %", mActor->getName(), e.what());
        }
    }

    void Component::triggerBegin(Actor* otherActor, Component* myComponent, Component* otherComponent)
    {
        try
        {
            onTriggerBegin(otherActor, myComponent, otherComponent);
        }
        catch (InvalidReference &e)
        {
            ERROR("OnTriggerBegin() error in one of %'s components: %", mActor->getName(), e.what());
        }
    }

    void Component::preRender()
    {
        try
        {
            onPreRender();
        }
        catch (InvalidReference &e)
        {
            ERROR("OnPreRender() error in one of %'s components: %", mActor->getName(), e.what());
        }
    }
    
    void Component::onPreRender()
    {
    
    }

    void Component::onCollisionBegin(
        Actor* otherActor, Component* myComponent, Component* otherComponent, const HitInfo& hitInfo)
    {
    }

    void Component::onTriggerBegin(Actor* otherActor, Component* myComponent, Component* otherComponent)
    {

    }

    Actor *Component::getActor() const
    {
        return mActor;
    }

    void Component::onAwake()
    {

    }

    void Component::onBegin()
    {
    
    }
}
