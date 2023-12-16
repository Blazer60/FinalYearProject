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

#include "EngineRandom.h"
#include "HitInfo.h"
#include "Scene.h"

namespace load
{
    void actor(const YAML::Node&, engine::Scene*);
    std::unique_ptr<class engine::Scene> scene(const std::filesystem::path &path);
}

namespace engine
{
    /**
     * An Actor is a game object that can live in a scene. Actors have components that can be attached to them.
     * @author Ryan Purse
     * @date 07/08/2023
     */
    class Actor
        : public ui::Drawable
    {
        friend void serialize::actor(YAML::Emitter &out, Actor *actor);
        friend void load::actor(const YAML::Node&, engine::Scene*);
        friend std::unique_ptr<Scene> load::scene(const std::filesystem::path &);
    public:
        friend class Scene;
        Actor() = default;
        explicit Actor(std::string name);
        ~Actor() override = default;

        void awake();
        void begin();
        void update();
        void fixedUpdate();
        void collisionBegin(Actor *otherActor, Component *myComponent, Component *otherComponent, const HitInfo &hitInfo);
        void triggerBegin(Actor *otherActor, Component *myComponent, Component *otherComponent);

        /**
         * @returns The ID of the current actor.
         */
        [[nodiscard]] UUID getId() const;
        
    
        /**
         * @returns The name of the Acotr. This is the name that is shown in the hierarchy.
         */
        [[nodiscard]] std::string_view getName() const;
        
        /**
         * @returns All of the components that are attached to this Actor.
         */
        [[nodiscard]] std::vector<Resource<Component>> &getComponents();
        
        /**
         * @brief Adds the component the the list of components attached to this Actor.
         */
        template<typename T>
        Ref<T> addComponent(Resource<T> &&component);
        
        /**
         * @returns The transform of this Actor in world space.
         */
        [[nodiscard]] glm::mat4 getTransform() const;

        /**
         * @returns The transform of this actor in object space.
         */
        [[nodiscard]] glm::mat4 getLocalTransform() const;
        
        /**
         * @tparam T - The type of serializeComponent.
         * @returns The first serializeComponent of type T or a subclass of T, Nullptr otherwise.
         */
        template<typename T>
        Ref<T> getComponent(bool warn=true);
        
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

        /**
         * \brief Destroys a component attached to this actor.
         * \tparam T The component type that you want to remove.
         */
        template<typename T>
        void removeComponent();

        /**
         * \brief Destroys a component attached to this actor.
         * \param component The component that you want to remove.
         */
        void removeComponent(const Component *component);

        /**
         * \brief Destroys a component attached to this actor.
         * \param component The component that you want to remove.
         */
        void removeComponent(const Ref<Component> &component);

        /**
         * \brief Adds a child actor to this actor.
         * \tparam TActor The type of actor. It must inherit from actor.
         * \param actor The child Actor that you want to add.
         * \param keepWorldRelative Should the child actor's transform be updated so the it's world space transform does not update
         * \returns - The actor passed in (for chaining).
         */
        template<typename TActor, std::enable_if_t<std::is_convertible_v<TActor*, Actor*>, bool> = true>
        Ref<TActor> addChildActor(Ref<TActor> actor, bool keepWorldRelative=true);

        /**
         * \brief Adds a child actor to this actor.
         * \tparam TActor The type of actor. It must inheir from actor.
         * \param actor The child actor that you want to add.
         * \returns - The actor resource passed in but as a reference (for chainging).
         */
        template<typename TActor, std::enable_if_t<std::is_convertible_v<TActor*, Actor*>, bool> = true>
        Ref<TActor> addChildActor(Resource<TActor> actor);

        /**
         * \brief Remvoes a child actor from this actor.
         */
        void removeChildActor(Actor* actor);
        
        [[nodiscard]] std::vector<UUID> &getChildren();
        [[nodiscard]] Actor *getParent() const;

        void setWorldTransform(const glm::mat4 &worldTransform);
        [[nodiscard]] glm::vec3 getWorldPosition() const;
        [[nodiscard]] glm::quat getWorldRotation() const;
        [[nodiscard]] Scene *getScene() const;

    protected:
        void onDrawUi() override;

        /**
         * \brief Uses this function to initialise any interal state within the actor. Guarenteed to be called before begin().
         */
        virtual void onAwake();

        /**
         * \brief Use this function to find other components that you want to attach to. Guarenteed to be called before update().
         */
        virtual void onBegin();

        /**
         * \brief Called every frame.
         */
        virtual void onUpdate();

        /**
         * \brief Called when an actor has both a rigid body and a collider and isTrigger is set to false.
         * \param otherActor The other actor that this collided with
         * \param myComponent The component that caused the collision event
         * \param otherComponent The other component that we came into contact with.
         * \param hitInfo More information about the hit.
         */
        virtual void onCollisionBegin(Actor *otherActor, Component *myComponent, Component *otherComponent, const HitInfo &hitInfo);

        /**
         * \brief Called when an actor has both arigid body and a collider and isTrigger is set to true.
         * \param otherActor The other actor that this collided with
         * \param myComponent The component that cuased the collision event
         * \param otherComponent The other component that we came into contact with.
         */
        virtual void onTriggerBegin(Actor *otherActor, Component *myComponent, Component *otherComponent);

    private:
        void updateTransform();
        void updateComponents();

    protected:
        std::string mName      { "Actor" };  // I've put the name here so that the debugger shows this as the first field.
        Scene *mScene    { nullptr };
        UUID mId               { random::generateId() };

    public:
        glm::vec3 position     { glm::vec3(0.f) };
        glm::quat rotation     { glm::identity<glm::quat>() };
        glm::vec3 scale        { glm::vec3(1.f) };

    protected:
        glm::mat4         mTransform    { glm::mat4(1.f) };
        Actor*            mParent       { nullptr };  // Nullptr means that its parent is the scene.
        std::vector<UUID> mChildren;

        std::vector<Resource<Component>> mComponents;
    private:
        std::set<const Component*>  mComponentDestroyBuffer0;
        std::set<const Component*>  mComponentDestroyBuffer1;
        std::set<const Component*> *mComponentsToDestroy { &mComponentDestroyBuffer0 };

        std::vector<Resource<Component>> mComponentsToAdd;
        std::set<UUID>                   mChildrenToRemove;
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
    Ref<T> Actor::getComponent(const bool warn)
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
                if (warn)
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
        const bool inComponentList = std::any_of(mComponents.begin(), mComponents.end(), [](const Resource<Component> &component) {
            const T* t = dynamic_cast<const T*>(component.get());
            if (t == nullptr)
                return false;
            else  // Make sure that it's the exact type rather than a child type.
                return typeid(const T*).hash_code() == typeid(t).hash_code();
        });
        
        const bool inAddList = std::any_of(mComponentsToAdd.begin(), mComponentsToAdd.end(), [](const Resource<Component> &component) {
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
    Ref<TActor> Actor::addChildActor(Ref<TActor> tActor, const bool keepWorldRelative)
    {
        Ref<Actor> actor(tActor);

        actor->mParent = this;
        actor->mScene = mScene;  // In case this wasn't created using spawnActor<>();

        if (keepWorldRelative)
        {
            actor->mTransform = glm::inverse(getTransform()) * actor->mTransform;
            math::decompose(actor->mTransform, actor->position, actor->rotation, actor->scale);
        }

        mChildren.push_back(actor->getId());

        return actor;
    }

    template<typename TActor, std::enable_if_t<std::is_convertible_v<TActor*, Actor*>, bool>>
    Ref<TActor> Actor::addChildActor(Resource<TActor> actor)
    {
        return addChildActor(mScene->addActor(std::move(actor)));
    }
} // engine
