/**
 * @file MyTestActor.cpp
 * @author Ryan Purse
 * @date 29/10/2023
 */


#include "MyTestActor.h"


MyTestActor::MyTestActor(std::string name)
    : engine::Actor(std::move(name))
{

}

void MyTestActor::onBegin()
{
    Ref<engine::Actor> a = addChildActor(makeResource<engine::Actor>("Should be dead."));
    a->markForDeath();
}

void MyTestActor::onUpdate()
{
    mTimer += timers::deltaTime<float>();
    if (mTimer > 0.001f)
    {
        mTimer -= 0.001f;
        addChildActor(makeResource<engine::Actor>("Child"));

        if (mChildren.size() > 100)
            mChildren.clear();
    }
}
