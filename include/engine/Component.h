/**
 * @file Component.h
 * @author Ryan Purse
 * @date 07/08/2023
 */


#pragma once

#include "Pch.h"
#include "Drawable.h"
#include "HitInfo.h"
#include "Serializer.h"

namespace engine
{
/**
 * @author Ryan Purse
 * @date 07/08/2023
 */
    class Component
        : public ui::Drawable
    {
    public:
        ~Component() override = default;
        void begin();
        void update();
        void fixedUpdate();
        void preRender();
        void attachToActor(class Actor *actor);
        void collisionBegin(Actor *otherActor, Component *myComponent, Component *otherComponent, const HitInfo &hitInfo);
        void triggerBegin(Actor *otherActor, Component *myComponent, Component *otherComponent);
        [[nodiscard]] Actor *getActor() const;
        
    protected:
        virtual void onBegin();
        virtual void onUpdate();
        virtual void onFixedUpdate();
        virtual void onPreRender();
        virtual void onCollisionBegin(Actor *otherActor, Component *myComponent, Component *otherComponent, const HitInfo &hitInfo);
        virtual void onTriggerBegin(Actor *otherActor, Component *myComponent, Component *otherComponent);
        void onDrawUi() override;
        [[nodiscard]] glm::mat4 getWorldTransform() const;
        
    protected:
        Actor *mActor;
    };
    
} // engine
