/**
 * @file Scene.h
 * @author Ryan Purse
 * @date 13/06/2023
 */


#pragma once

#include "Pch.h"
#include "TextureBufferObject.h"
#include "Renderer.h"
#include "Actor.h"
#include "EngineMemory.h"
#include "Callback.h"

namespace load
{
    void scene(const std::filesystem::path &path, engine::Scene *scene);
}

namespace engine
{

    /**
     * @author Ryan Purse
     * @date 13/06/2023
     */
    class Scene
    {
        friend void load::scene(const std::filesystem::path &, engine::Scene *);
    public:
        Callback<Ref<Actor>> onDeath;
        
        virtual ~Scene() = default;
        
        void update();
        void render();
        void imguiUpdate();
        std::vector<Resource<Actor>> &getActors();
        
        /**
         * @brief Actors are destroyed at the end of the update loop.
         */
        void destroy(const Actor* actor);
        
        virtual void onFixedUpdate();
        virtual void onRender();
        virtual void onImguiMenuUpdate();
        
        template<typename TActor, typename ...TArgs>
        Ref<TActor> spawnActor(TArgs &&... args);
        
        template<typename TActor, std::enable_if_t<std::is_convertible_v<TActor*, Actor*>, bool> = true>
        Ref<TActor> addActor(Resource<TActor> &&actor);

        Ref<Actor> getActor(UUID actorId) const;
        
    protected:
        virtual void onUpdate();
        virtual void onImguiUpdate();


        std::set<const Actor*> mDestroyBuffer0;
        std::set<const Actor*> mDestroyBuffer1;
        std::set<const Actor*> *mToDestroy { &mDestroyBuffer0 };
        
        std::vector<Resource<Actor>> mToAdd;
        
        std::vector<Resource<Actor>> mActors;
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
        mToAdd.push_back(std::move(actor));
        
        return actorRef;
    }
} // engine
