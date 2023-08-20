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
    
        /**
         * @returns The name of the actor. This is the name that is shown in the hierarchy.
         */
        [[nodiscard]] std::string_view getName() const;
        
        /**
         * @returns All of the components that are attached to this actor.
         */
        [[nodiscard]] std::vector<std::unique_ptr<Component>> &getComponents();
        
        /**
         * @brief Adds the component the the list of components attached to this actor.
         */
        void addComponent(std::unique_ptr<Component> component);
        
        /**
         * @returns The transform of this actor in world space.
         */
        [[nodiscard]] glm::mat4 getTransform() const;
        
        /**
         * @tparam T - The type of component.
         * @returns The first component of type T or a subclass of T, Nullptr otherwise.
         */
        template<typename T>
        T* getComponent();
        
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
    
    template<typename T>
    T* Actor::getComponent()
    {
        for (auto &component : mComponents)
        {
            if (T* t = dynamic_cast<T*>(component.get()); t != nullptr)
                return t;
        }
        return nullptr;
    }
} // engine
