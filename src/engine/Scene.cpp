/**
 * @file Scene.cpp
 * @author Ryan Purse
 * @date 13/06/2023
 */


#include "Scene.h"
#include "imgui.h"
#include "WindowHelpers.h"
#include "ProfileTimer.h"

namespace engine
{
    void Scene::update()
    {
        PROFILE_FUNC();
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
        PROFILE_FUNC();
        onImguiUpdate();
    }
    
    std::vector<Resource<Actor>> &Scene::getActors()
    {
        return mActors;
    }
    
    void Scene::render()
    {
        PROFILE_FUNC();
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
    
    Resource<Actor> Scene::popActor(Actor *actor)
    {
        const auto it = std::find_if(mActors.begin(), mActors.end(), [&actor](const Resource<Actor> &child) {
            return child.get() == actor;
        });
        
        if (it == mActors.end())
        {
            LOG_MAJOR("Failed to find actor while popping");
            return Resource<Actor>();
        }
        
        Resource<Actor> out = std::move(*it);
        mActors.erase(it);
        
        return out;
    }
    
}
