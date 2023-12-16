/**
 * @file Component.h
 * @author Ryan Purse
 * @date 07/08/2023
 */


#pragma once

#include "Pch.h"
#include "Drawable.h"
#include "HitInfo.h"

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
        void awake();
        void begin();
        void update();
        void fixedUpdate();
        void preRender();
        void attachToActor(Actor *actor);
        void collisionBegin(Actor *otherActor, Component *myComponent, Component *otherComponent, const HitInfo &hitInfo);
        void triggerBegin(Actor *otherActor, Component *myComponent, Component *otherComponent);
        [[nodiscard]] Actor *getActor() const;
        
    protected:
        /**
         * \brief Use this function to initialise any internal state within the component. Guarenteed to be called before onBegin();
         */
        virtual void onAwake();

        /**
         * \brief Use this function to find other components on the acotr. Guarenteed to be called before onUpdate();
         */
        virtual void onBegin();

        /**
         * \brief Called every frame.
         */
        virtual void onUpdate();

        /**
         * \brief Called after every physics update.
         */
        virtual void onFixedUpdate();

        /**
         * \brief Called before the scene is rendered. Use this step to submit things to the renderer.
         */
        virtual void onPreRender();

        /**
         * \brief Called when the attatched actor has both a rigid body and a collider and isTrigger is set to false.
         * \param otherActor The other actor that this collided with.
         * \param myComponent The component that caused the collision event.
         * \param otherComponent The other component that we came into comntat with.
         * \param hitInfo More information about the hit.
         */
        virtual void onCollisionBegin(Actor *otherActor, Component *myComponent, Component *otherComponent, const HitInfo &hitInfo);

        /**
         * \brief Called when the attached actor has both a rigid body and a collider and isTrigger is set to true.
         * \param otherActor the other actor that caused the collision event.
         * \param myComponent The component that caused the collision event.
         * \param otherComponent The other component that we came into contact with.
         */
        virtual void onTriggerBegin(Actor *otherActor, Component *myComponent, Component *otherComponent);

        /**
         * \brief This can be override to draw your own custom property drawer. If left blank, nothing will be drawn.
         */
        void onDrawUi() override;

        [[nodiscard]] glm::mat4 getWorldTransform() const;
        
        Actor *mActor { nullptr };
    };
    
} // engine
