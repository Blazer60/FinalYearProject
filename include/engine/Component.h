/**
 * @file Component.h
 * @author Ryan Purse
 * @date 07/08/2023
 */


#pragma once

#include "Pch.h"
#include "Drawable.h"
#include "SerializeMacros.h"

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
        virtual ~Component() = default;
        void update();
        void preRender();
        void attachToActor(class Actor *actor);
        [[nodiscard]] class Actor *getActor() const;
        
    protected:
        virtual void onUpdate();
        virtual void onPreRender();
        void onDrawUi() override;
        [[nodiscard]] glm::mat4 getWorldTransform() const;
        
    protected:
        class Actor *mActor;
    };
    
} // engine
