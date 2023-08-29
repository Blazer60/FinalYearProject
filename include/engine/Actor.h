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
#include "EngineMemory.h"

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
        friend class Scene;
        Actor() = default;
        explicit Actor(std::string name);
        virtual ~Actor()
        {
            MESSAGE("Deleting Actor.");
        }
        
        void update();
        
    
        /**
         * @returns The name of the actor. This is the name that is shown in the hierarchy.
         */
        [[nodiscard]] std::string_view getName() const;
        
        /**
         * @returns All of the components that are attached to this actor.
         */
        [[nodiscard]] std::vector<Resource<Component>> &getComponents();
        
        /**
         * @brief Adds the component the the list of components attached to this actor.
         */
        Ref<Component> addComponent(Resource<Component> &&component);
        
        /**
         * @returns The transform of this actor in world space.
         */
        [[nodiscard]] glm::mat4 getTransform() const;
        
        /**
         * @tparam T - The type of component.
         * @returns The first component of type T or a subclass of T, Nullptr otherwise.
         */
        template<typename T>
        Ref<Component> getComponent();
        
        /**
         * @tparam T - the type of component.
         * @returns True if it could find a component with type T, False otherwise.
         */
        template<typename T>
        [[nodiscard]] bool hasComponent() const;
        
        /**
         * @brief This actor will be destroyed at the end of the update loop.
         */
        void markForDeath();
        
        template<typename T>
        void removeComponent();
        
        void removeComponent(Component *component);
        
    public:
        glm::vec3 position     { glm::vec3(0.f) };
        glm::quat rotation     { glm::identity<glm::quat>() };
        glm::vec3 scale        { glm::vec3(1.f) };
        
    protected:
        void onDrawUi() override;
        void updateTransform();
        virtual void onUpdate();
        
        std::string mName       { "Actor" };
        glm::mat4 mTransform    { glm::mat4(1.f) };
        
        std::vector<Resource<Component>> mComponents;
        class Scene *mScene;
        
    private:
        std::set<uint32_t> mToDestroy;
    };
    
    template<typename T>
    Ref<Component> Actor::getComponent()
    {
        for (auto &component : mComponents)
        {
            if (T* t = dynamic_cast<T*>(component.get()); t != nullptr)
                return t;
        }
        return Ref<Component>();
    }
    
    template<typename T>
    bool Actor::hasComponent() const
    {
        return std::any_of(mComponents.begin(), mComponents.end(), [](const auto &component) {
            const T* t = dynamic_cast<const T*>(component.get());
            if (t == nullptr)
                return false;
            else  // Make sure that it's the exact type rather than a child type.
                return typeid(T*).hash_code() == typeid(t).hash_code();
        });
    }
    
    template<typename T>
    void Actor::removeComponent()
    {
        const auto it = std::find_if(mComponents.begin(), mComponents.end(), [](const std::unique_ptr<Component> &component) {
            T* t = dynamic_cast<T*>(component.get());
            if (t == nullptr)
                return false;
            else  // Make sure that it's the exact type rather than a child type.
                return typeid(T*).hash_code() == typeid(t).hash_code();
        });
        
        if (it == mComponents.end())
        {
            WARN("Component % does not exist in this actor and so it cannot be removed.", typeid(T).name());
            return;
        }
        
        const uint32_t index = std::distance(mComponents.begin(), it);
        
        mToDestroy.emplace(index);
    }
} // engine
