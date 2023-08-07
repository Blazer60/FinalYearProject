/**
 * @file Actor.h
 * @author Ryan Purse
 * @date 07/08/2023
 */


#pragma once

#include "Pch.h"
#include "detail/type_quat.hpp"
#include "Drawable.h"

namespace engine
{
    /**
     * An actor is a game object that can live in a scene. Actors have components that can be attached to them.
     * @author Ryan Purse
     * @date 07/08/2023
     */
    class Actor
        : public ui::Drawable
    {
    public:
        virtual ~Actor() = default;
        
        virtual void OnUpdate();
    
        [[nodiscard]] std::string_view getName() const;
        
    protected:
        void onDrawUi() override;
        
        std::string mName       { "Actor" };
        glm::mat4 mTransform    { glm::mat4(1.f) };
        glm::vec3 mPosition     { glm::vec3(0.f) };
        glm::quat mRotation     { glm::identity<glm::quat>() };
        glm::vec3 mScale        { glm::vec3(1.f) };
    };
    
} // engine
