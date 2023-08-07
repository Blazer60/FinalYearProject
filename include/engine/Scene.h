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
        
        virtual void onFixedUpdate();
        virtual void onRender();
        virtual void onImguiMenuUpdate();
        
    protected:
        virtual void onUpdate();
        virtual void onImguiUpdate();
        
        std::vector<std::unique_ptr<Actor>> mActors;
        Actor *mSelectedActor { nullptr };
    };
    
} // engine
