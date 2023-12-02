/**
 * @file MyTestActor.cpp
 * @author Ryan Purse
 * @date 29/10/2023
 */


#include "MyTestActor.h"

#include "Scene.h"


MyTestActor::MyTestActor(std::string name)
    : engine::Actor(std::move(name))
{

}

void MyTestActor::onBegin()
{
    auto spawnedActor = getScene()->spawnActor<engine::Actor>("Should be dead.");
    Ref<engine::Actor> a = addChildActor(spawnedActor);
    a->markForDeath();
}

void MyTestActor::onUpdate()
{
    mTimer += timers::deltaTime<float>();
    if (mTimer > 0.001f)
    {
        mTimer -= 0.001f;
        std::string name = "Child" + std::to_string(mCount++);
        addChildActor(makeResource<Actor>(name));

        if (mChildren.size() > 5)
        {
            for (const engine::UUID child : mChildren)
                getScene()->getActor(child)->markForDeath();
            mChildren.clear();
        }
    }
}
