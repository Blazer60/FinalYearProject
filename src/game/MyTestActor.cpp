/**
 * @file MyTestActor.cpp
 * @author Ryan Purse
 * @date 29/10/2023
 */


#include "MyTestActor.h"

#include <Engine.h>


MyTestActor::MyTestActor(std::string name)
    : Actor(std::move(name))
{

}

MyTestActor::MyTestActor(const float timer, const int count)
    : mTimer(timer), mCount(count)
{

}

void MyTestActor::onBegin()
{
    const Ref<Actor> spawnedActor = getScene()->spawnActor<Actor>("Should be dead.");
    Ref<Actor> a = addChildActor(spawnedActor);
    a->markForDeath();
}

void MyTestActor::onUpdate()
{
    mTimer += timers::deltaTime<float>();
    if (mTimer > 5.f)
    {
        mTimer -= 5.f;
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

void serializeActor(engine::serialize::Emitter &out, MyTestActor *const myTestActor)
{
    out << engine::serialize::Key << "Type"  << engine::serialize::Value << "MyTestActor";
    out << engine::serialize::Key << "Timer" << engine::serialize::Value << myTestActor->mTimer;
    out << engine::serialize::Key << "Count" << engine::serialize::Value << myTestActor->mCount;
}

