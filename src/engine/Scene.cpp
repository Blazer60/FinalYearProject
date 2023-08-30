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
            actor->update();
        
        for (const uint32_t index : mToDestroy)
        {
            onDeath.broadcast(mActors[index]);
            mActors.erase(mActors.begin() + index);
        }
        
        mToDestroy.clear();
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
    
    std::vector<Resource<Actor>> &Scene::getActors()
    {
        return mActors;
    }
    
    void Scene::render()
    {
        for (auto &actor : mActors)
            recursePreRender(actor);
        
        onRender();
    }
    
    void Scene::destroy(const Actor* actor)
    {
        const auto it = std::find_if(mActors.begin(), mActors.end(), [&actor](const Ref<Actor> &left) {
            return left.get() == actor;
        });
        
        if (it == mActors.end())
        {
            WARN("Actor % does not exist in this scene and so it cannot be removed.", actor->getName());
            return;
        }
        
        const uint32_t index = std::distance(mActors.begin(), it);
        
        mToDestroy.emplace(index);
    }
    
    void Scene::recursePreRender(Ref<Actor> actor)
    {
        for (Resource<Component> &component : actor->getComponents())
            component->preRender();
        
        for (Resource<Actor> &child : actor->getChildren())
            recursePreRender(child);
    }
    
}
