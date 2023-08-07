/**
 * @file Component.h
 * @author Ryan Purse
 * @date 07/08/2023
 */


#pragma once

#include "Pch.h"
#include "Drawable.h"

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
        void attachToActor(class Actor *actor);
        
    protected:
        virtual void onUpdate();
        void onDrawUi() override;
        [[nodiscard]] glm::mat4 getWorldTransform() const;
        
    protected:
        class Actor *mActor;
    };
    
} // engine
