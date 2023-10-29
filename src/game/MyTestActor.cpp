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

void MyTestActor::onUpdate()
{
    mTimer += timers::deltaTime<float>();
    if (mTimer > 0.1f)
    {
        mTimer -= 0.1f;
        addChildActor(makeResource<engine::Actor>("Child"));
        
        if (mChildren.size() > 10)
            mChildren.clear();
    }
}

