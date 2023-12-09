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
        WARN("The serializeComponent has not been attached to an serializeActor yet.");
        return glm::mat4(1.f);
    }
    
    void Component::attachToActor(Actor *actor)
    {
        mActor = actor;
    }

    void Component::collisionBegin(
        Actor* otherActor, Component* myComponent, Component* otherComponent, const HitInfo& hitInfo)
    {
        onCollisionBegin(otherActor, myComponent, otherComponent, hitInfo);
    }

    void Component::preRender()
    {
        onPreRender();
    }
    
    void Component::onPreRender()
    {
    
    }

    void Component::onCollisionBegin(
        Actor* otherActor, Component* myComponent, Component* otherComponent, const HitInfo& hitInfo)
    {
    }

    Actor *Component::getActor() const
    {
        return mActor;
    }
    
    void Component::onBegin()
    {
    
    }
}
