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

namespace engine
{

    /**
     * @author Ryan Purse
     * @date 13/06/2023
     */
    class Scene
    {
    public:
        Callback<Actor*> onDeath;
        
        virtual ~Scene() = default;
        
        void update();
        void render();
        void imguiUpdate();
        std::vector<std::unique_ptr<Actor>> &getActors();
        
        /**
         * @brief Actors are destroyed at the end of the update loop.
         */
        void destroy(Actor *actor);
        
        virtual void onFixedUpdate();
        virtual void onRender();
        virtual void onImguiMenuUpdate();
        
        template<typename TActor, typename ...TArgs>
        TActor *spawnActor(TArgs&&... args);
        
    protected:
        virtual void onUpdate();
        virtual void onImguiUpdate();
        
        std::vector<std::unique_ptr<Actor>> mActors;
        std::set<uint32_t> mToDestroy;
    };
    
    template<typename TActor, typename... TArgs>
    TActor *Scene::spawnActor(TArgs &&... args)
    {
        std::unique_ptr<TActor> actor = std::make_unique<TActor>(std::forward<TArgs>(args)...);
        TActor *actorRef = actor.get();
        
        actor->mScene = this;
        mActors.push_back(std::move(actor));
        
        return actorRef;
    }
} // engine
