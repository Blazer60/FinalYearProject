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
        
        for (int i = 0; i <mToAdd.size(); ++i)
        {
            mToAdd[i]->begin();
            mActors.push_back(std::move(mToAdd[i]));
        }
        mToAdd.clear();
        
        for (auto & actor : mActors)
            actor->update();
        
        auto currentDestroyBuffer = mToDestroy;
        if (currentDestroyBuffer == &mDestroyBuffer0)
            mToDestroy = &mDestroyBuffer1;
        else
            mToDestroy = &mDestroyBuffer0;
        
        for (const Actor *actor : *currentDestroyBuffer)
        {
            const auto it = std::find_if(mActors.begin(), mActors.end(), [&actor](const Ref<Actor> &left) {
                return left.get() == actor;
            });
            
            onDeath.broadcast(mActors[std::distance(mActors.begin(), it)]);
            mActors.erase(it);
        }
        
        currentDestroyBuffer->clear();
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
            const auto it2 = std::find_if(mToAdd.begin(), mToAdd.end(), [&actor](const Ref<Actor> &left) {
                return left.get() == actor;
            });
            
            if (it2 == mToAdd.end())
            {
                WARN("Actor % does not exist in this scene and so it cannot be removed.", actor->getName());
                return;
            }
        }
        
        mToDestroy->emplace(actor);
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
            LOG_MAJOR("Failed to find serializeActor while popping");
            return Resource<Actor>();
        }
        
        Resource<Actor> out = std::move(*it);
        mActors.erase(it);
        
        return out;
    }
}
