/**
 * @file Scene.cpp
 * @author Ryan Purse
 * @date 13/06/2023
 */


#include "Scene.h"
#include "imgui.h"
#include "WindowHelpers.h"

namespace engine
{
    void Scene::update()
    {
        onUpdate();
        
        for (auto &actor : mActors)
        {
            actor->OnUpdate();
            for (auto &component : actor->getComponents())
                component->update();
        }
    }

    void Scene::onFixedUpdate()
    {

    }

    void Scene::onUpdate()
    {

    }

    void Scene::onRender()
    {

    }

    void Scene::onImguiUpdate()
    {

    }

    void Scene::onImguiMenuUpdate()
    {

    }
    
    void Scene::imguiUpdate()
    {
        onImguiUpdate();
    }
    
    std::vector<std::unique_ptr<Actor>> &Scene::getActors()
    {
        return mActors;
    }
    
}
