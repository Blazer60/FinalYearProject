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
#include "Scene.h"

namespace load
{
    void actor(const YAML::Node&, engine::Scene*);
    void scene(const std::filesystem::path &path, engine::Scene *scene);
}

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
        friend void load::actor(const YAML::Node&, engine::Scene*);
        friend void load::scene(const std::filesystem::path &, engine::Scene *);
    public:
        friend class Scene;
        Actor() = default;
        explicit Actor(std::string name);
        ~Actor() override = default;
        
        void begin();
        void update();

        /**
         * @returns The ID of the current actor.
         */
        [[nodiscard]] UUID getId() const;
        
    
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
        Ref<TActor> addChildActor(Ref<TActor> actor, bool keepWorldRelative=true);

        template<typename TActor, std::enable_if_t<std::is_convertible_v<TActor*, Actor*>, bool> = true>
        Ref<TActor> addChildActor(Resource<TActor> actor);

        void removeChildActor(Actor* actor);
        
        [[nodiscard]] std::vector<UUID> &getChildren();
        [[nodiscard]] Actor *getParent() const;
        [[nodiscard]] glm::vec3 getWorldPosition() const;
        [[nodiscard]] Scene *getScene() const;
        
    protected:
        std::string mName      { "Actor" };  // I've put the name here so that the debugger shows this as the first field.
        class Scene *mScene    { nullptr };
        UUID mId               { random::generateId() };
    public:
        glm::vec3 position     { glm::vec3(0.f) };
        glm::quat rotation     { glm::identity<glm::quat>() };
        glm::vec3 scale        { glm::vec3(1.f) };
    
    private:
        std::set<const Component*>  mComponentDestroyBuffer0;
        std::set<const Component*>  mComponentDestroyBuffer1;
        std::set<const Component*> *mComponentsToDestroy { &mComponentDestroyBuffer0 };

        std::vector<Resource<Component>> mComponentsToAdd;
        std::set<UUID> mChildrenToRemove;
        
    protected:
        void onDrawUi() override;
        void updateTransform();
        virtual void onUpdate();
        virtual void onBegin();
        void updateComponents();
        
        glm::mat4 mTransform    { glm::mat4(1.f) };
        
        Actor*                           mParent { nullptr };  // Nullptr means that its parent is the scene.
        std::vector<UUID>                mChildren;
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
