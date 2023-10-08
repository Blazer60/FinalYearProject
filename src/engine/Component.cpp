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
    void Component::update()
    {
        onUpdate();
    }
    
    void Component::onUpdate()
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
    
    void Component::preRender()
    {
        onPreRender();
    }
    
    void Component::onPreRender()
    {
    
    }
    
    Actor *Component::getActor() const
    {
        return mActor;
    }
}
