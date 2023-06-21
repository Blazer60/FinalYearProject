/**
 * @file Scene.h
 * @author Ryan Purse
 * @date 13/06/2023
 */


#pragma once

#include "Pch.h"
#include "TextureBufferObject.h"

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
        
        virtual void onFixedUpdate();
        virtual void onUpdate();
        virtual void onRender();
        virtual void onImguiUpdate();
        virtual void onImguiMenuUpdate();
        
    protected:
        static void showTextureBuffer(const std::string &name, const TextureBufferObject &textureBufferObject, bool *show);
    };
    
} // engine
