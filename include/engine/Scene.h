/**
 * @file Scene.h
 * @author Ryan Purse
 * @date 13/06/2023
 */


#pragma once

#include "Pch.h"
#include "Actor.h"
#include "EngineMemory.h"
#include "Callback.h"

namespace load
{
    std::unique_ptr<engine::Scene> scene(const std::filesystem::path &path);
}

namespace engine
{

    /**
     * @author Ryan Purse
     * @date 13/06/2023
     */
    class Scene
        : public ui::Drawable
    {
        friend std::unique_ptr<Scene> load::scene(const std::filesystem::path &);
    public:
        ~Scene() override = default;
        
        void update();
        void fixedUpdate();
        void preRender();

        /**
         * \returns A list of actors that can be iterated over.
         */
        std::vector<Resource<Actor>> &getActors();
        
        /**
         * @brief Actors are destroyed at the end of the update loop.
         */
        void destroy(const Actor* actor);

        /**
         * \briefs Spawns an actor into the world.
         * \tparam TActor The type of actor that you want to spawn.
         * \tparam TArgs The type of arguments to forwrd to the actor's constructor.
         * \param args The arguments you want to forward to the actor's constructor.
         * \return
         */
        template<typename TActor, typename ...TArgs>
        Ref<TActor> spawnActor(TArgs &&... args);

        /**
         * \brief Adds an actor the scene.
         * \tparam TActor The type of actor that you want to add.
         * \param actor The actor you want to add.
         * \returns A reference to the actor.
         */
        template<typename TActor, std::enable_if_t<std::is_convertible_v<TActor*, Actor*>, bool> = true>
        Ref<TActor> addActor(Resource<TActor> &&actor);

        /**
         * \brief Finds the actor via the given id.
         * \param actorId The ID of the actor that you want to find.
         * \param warn Should a warning message be printed to the console if it can't find the specified actor
         * \returns A reference to the actor. A nullreference if it could not find anything.
         */
        Ref<Actor> getActor(UUID actorId, bool warn=true) const;

        /**
         * @brief Finds all components of type T that exist within the scene. This is a slow linear search. Do not use every frame!
         */
        template<typename T>
        std::vector<Ref<T>> findComponents();

    protected:
        virtual void onUpdate();
        virtual void onFixedUpdate();
        virtual void onPreRender();
        void onDrawUi() override;

    public:
        std::vector<Resource<Actor>> mActors;

    private:
        std::set<const Actor*> mDestroyBuffer0;
        std::set<const Actor*> mDestroyBuffer1;
        std::set<const Actor*> *mToDestroy { &mDestroyBuffer0 };
        
        std::vector<Resource<Actor>> mToAdd;
    };
    
    template<typename TActor, typename... TArgs>
    Ref<TActor> Scene::spawnActor(TArgs &&... args)
    {
        return addActor<TActor>(makeResource<TActor>(std::forward<TArgs>(args)...));
    }
    
    template<typename TActor, std::enable_if_t<std::is_convertible_v<TActor *, Actor *>, bool>>
    Ref<TActor> Scene::addActor(Resource<TActor> &&actor)
    {
        Ref<TActor> actorRef = actor;
        
        actor->mScene = this;
        actor->mParent = nullptr;
        actor->awake();
        mToAdd.push_back(std::move(actor));
        
        return actorRef;
    }

    template<typename T>
    [[nodiscard]] std::vector<Ref<T>> Scene::findComponents()
    {
        std::vector<Ref<T>> results;

        for (Ref<Actor> actor : mActors)
        {
            if (Ref<T> component = actor->getComponent<T>(false); component.isValid())
                results.push_back(component);
        }

        for (Ref<Actor> actor : mToAdd)
        {
            if (Ref<T> component = actor->getComponent<T>(false); component.isValid())
                results.push_back(component);
        }

        return results;
    }
} // engine
