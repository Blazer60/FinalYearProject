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
#include "EngineMath.h"
#include "Serializer.h"
#include <yaml-cpp/emitter.h>

namespace engine
{
    /**
     * An serializeActor is a game object that can live in a scene. Actors have components that can be attached to them.
     * @author Ryan Purse
     * @date 07/08/2023
     */
    class Actor
        : public ui::Drawable
    {
        friend void serialize::actor(YAML::Emitter &out, Actor *actor);
    public:
        friend class Scene;
        Actor() = default;
        explicit Actor(std::string name);
        ~Actor() override = default;
        
        void update();
        
    
        /**
         * @returns The name of the serializeActor. This is the name that is shown in the hierarchy.
         */
        [[nodiscard]] std::string_view getName() const;
        
        /**
         * @returns All of the components that are attached to this serializeActor.
         */
        [[nodiscard]] std::vector<Resource<Component>> &getComponents();
        
        /**
         * @brief Adds the serializeComponent the the list of components attached to this serializeActor.
         */
        template<typename T>
        Ref<T> addComponent(Resource<T> &&component);
        
        /**
         * @returns The transform of this serializeActor in world space.
         */
        [[nodiscard]] glm::mat4 getTransform() const;
        
        [[nodiscard]] glm::mat4 getLocalTransform() const;
        
        /**
         * @tparam T - The type of serializeComponent.
         * @returns The first serializeComponent of type T or a subclass of T, Nullptr otherwise.
         */
        template<typename T>
        Ref<T> getComponent();
        
        /**
         * @tparam T - the type of serializeComponent.
         * @returns True if it could find a serializeComponent with type T, False otherwise.
         */
        template<typename T>
        [[nodiscard]] bool hasComponent() const;
        
        /**
         * @brief This serializeActor will be destroyed at the end of the update loop.
         */
        void markForDeath();
        
        template<typename T>
        void removeComponent();
        
        void removeComponent(const Component *component);
        void removeComponent(const Ref<Component> &component);
        
        template<typename TActor, std::enable_if_t<std::is_convertible_v<TActor*, Actor*>, bool> = true>
        Ref<TActor> addChildActor(Resource<TActor> &&actor);
        
        void removeChildActor(const Actor* actor);
        
        [[nodiscard]] std::vector<Resource<Actor>> &getChildren();
        [[nodiscard]] Actor *getParent();
        [[nodiscard]] glm::vec3 getWorldPosition();
        Resource<Actor> popActor(Actor *actor);
        [[nodiscard]] Scene *getScene();
        
    protected:
        std::string mName      { "Actor" };  // I've put the name here so that the debugger shows this as the first field.
        class Scene *mScene    { nullptr };
    public:
        glm::vec3 position     { glm::vec3(0.f) };
        glm::quat rotation     { glm::identity<glm::quat>() };
        glm::vec3 scale        { glm::vec3(1.f) };
    
    private:
        std::set<const Component*>  mComponentDestroyBuffer0;
        std::set<const Component*>  mComponentDestroyBuffer1;
        std::set<const Component*> *mComponentsToDestroy { &mComponentDestroyBuffer0 };
        
        std::set<const Actor*>  mActorDestroyBuffer0;
        std::set<const Actor*>  mActorDestroyBuffer1;
        std::set<const Actor*> *mActorsToDestroy { &mActorDestroyBuffer0 };
        
        std::vector<Resource<Actor>> mActorsToAdd;
        std::vector<Resource<Component>> mComponentsToAdd;
        
    protected:
        void onDrawUi() override;
        void updateTransform();
        virtual void onUpdate();
        void updateComponents();
        
        glm::mat4 mTransform    { glm::mat4(1.f) };
        
        Actor*                           mParent { nullptr };  // Nullptr means that its parent is the scene.
        std::vector<Resource<Actor>>     mChildren;
        std::vector<Resource<Component>> mComponents;
    };
    
    template<typename T>
    Ref<T> Actor::addComponent(Resource<T> &&component)
    {
        Ref<T> ref = component;
        component->attachToActor(this);
        mComponentsToAdd.push_back(std::move(component));
        return ref;
    }
    
    template<typename T>
    Ref<T> Actor::getComponent()
    {
        const auto it = std::find_if(mComponents.begin(), mComponents.end(), [](const Resource<Component> &component) {
            const T* t = dynamic_cast<const T*>(component.get());
            if (t == nullptr)
                return false;
            else  // Make sure that it's the exact type rather than a child type.
                return typeid(const T*).hash_code() == typeid(t).hash_code();
        });
        
        if (it == mComponents.end())
        {
            const auto it2 = std::find_if(mComponentsToAdd.begin(), mComponentsToAdd.end(), [](const Resource<Component> &component) {
                const T* t = dynamic_cast<const T*>(component.get());
                if (t == nullptr)
                    return false;
                else  // Make sure that it's the exact type rather than a child type.
                    return typeid(const T*).hash_code() == typeid(t).hash_code();
            });
            
            if (it2 == mComponentsToAdd.end())
            {
                WARN("Component % does not exist.", typeid(T).name());
                return Ref<T>();
            }
            
            return dynamic_ref_cast<T>(*it2);
        }
        
        return dynamic_ref_cast<T>(*it);
    }
    
    template<typename T>
    bool Actor::hasComponent() const
    {
        bool inComponentList = std::any_of(mComponents.begin(), mComponents.end(), [](const Resource<Component> &component) {
            const T* t = dynamic_cast<const T*>(component.get());
            if (t == nullptr)
                return false;
            else  // Make sure that it's the exact type rather than a child type.
                return typeid(const T*).hash_code() == typeid(t).hash_code();
        });
        
        bool inAddList = std::any_of(mComponentsToAdd.begin(), mComponentsToAdd.end(), [](const Resource<Component> &component) {
            const T* t = dynamic_cast<const T*>(component.get());
            if (t == nullptr)
                return false;
            else  // Make sure that it's the exact type rather than a child type.
                return typeid(const T*).hash_code() == typeid(t).hash_code();
        });
        
        return inComponentList || inAddList;
    }
    
    template<typename T>
    void Actor::removeComponent()
    {
        const auto it = std::find_if(mComponents.begin(), mComponents.end(), [](const Resource<Component> &component) {
            const T* t = dynamic_cast<const T*>(component.get());
            if (t == nullptr)
                return false;
            else  // Make sure that it's the exact type rather than a child type.
                return typeid(const T*).hash_code() == typeid(t).hash_code();
        });
        
        if (it == mComponents.end())
        {
            const auto it2 = std::find_if(mComponentsToAdd.begin(), mComponentsToAdd.end(), [](const Resource<Component> &component) {
                const T* t = dynamic_cast<const T*>(component.get());
                if (t == nullptr)
                    return false;
                else  // Make sure that it's the exact type rather than a child type.
                    return typeid(const T*).hash_code() == typeid(t).hash_code();
            });
            
            if (it2 == mComponentsToAdd.end())
            {
                WARN("Component % does not exist in this Actor and so it cannot be removed.", typeid(T).name());
                return;
            }
            
            mComponentsToAdd.erase(it2);
            return;
        }
        else
            mComponentsToDestroy->emplace((*it).get());
    }
    
    
    template<typename TActor, std::enable_if_t<std::is_convertible_v<TActor*, Actor*>, bool>>
    Ref<TActor> Actor::addChildActor(Resource<TActor> &&actor)
    {
        Ref<TActor> ref = actor;  // Getting a ref<> at the start to not lose the type.
        
        Resource<Actor> tempActor = std::move(actor);
        tempActor->mParent = this;
        tempActor->mScene = mScene;  // In case this wasn't created using spawnActor<>();
        
        tempActor->mTransform = glm::inverse(getTransform()) * tempActor->mTransform;
        math::decompose(tempActor->mTransform, tempActor->position, tempActor->rotation, tempActor->scale);
        
        mActorsToAdd.push_back(std::move(tempActor));
        
        return ref;
    }
} // engine
