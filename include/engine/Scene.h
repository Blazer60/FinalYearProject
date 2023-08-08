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
        virtual ~Scene() = default;
        
        void update();
        void imguiUpdate();
        std::vector<std::unique_ptr<Actor>> &getActors();
        
        virtual void onFixedUpdate();
        virtual void onRender();
        virtual void onImguiMenuUpdate();
        
    protected:
        virtual void onUpdate();
        virtual void onImguiUpdate();
        
        template<typename TActor, typename ...TArgs>
        TActor *spawnActor(TArgs&&... args);
        
        std::vector<std::unique_ptr<Actor>> mActors;
        Actor *mSelectedActor { nullptr };
    };
    
    template<typename TActor, typename... TArgs>
    TActor *Scene::spawnActor(TArgs &&... args)
    {
        std::unique_ptr<TActor> actor = std::make_unique<TActor>(std::forward<TArgs>(args)...);
        TActor *actorRef = actor.get();
        
        mActors.push_back(std::move(actor));
        
        return actorRef;
    }
} // engine
