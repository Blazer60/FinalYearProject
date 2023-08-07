/**
 * @file Actor.h
 * @author Ryan Purse
 * @date 07/08/2023
 */


#pragma once

#include "Pch.h"
#include "detail/type_quat.hpp"
#include "Drawable.h"
#include "Component.h"

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
        Actor() = default;
        explicit Actor(std::string name);
        virtual ~Actor() = default;
        
        virtual void OnUpdate();
    
        [[nodiscard]] std::string_view getName() const;
        [[nodiscard]] std::vector<std::unique_ptr<Component>> &getComponents();
        void addComponent(std::unique_ptr<Component> component);
        [[nodiscard]] glm::mat4 getTransform() const;
        
    public:
        glm::vec3 position     { glm::vec3(0.f) };
        glm::quat rotation     { glm::identity<glm::quat>() };
        glm::vec3 scale        { glm::vec3(1.f) };
        
    protected:
        void onDrawUi() override;
        void updateTransform();
        
        std::string mName       { "Actor" };
        glm::mat4 mTransform    { glm::mat4(1.f) };
        
        std::vector<std::unique_ptr<Component>> mComponents;
    };
    
} // engine
