/**
 * @file MyTestActor.cpp
 * @author Ryan Purse
 * @date 29/10/2023
 */


#include "MyTestActor.h"

#include <Engine.h>


MyTestActor::MyTestActor(std::string name)
    : engine::Actor(std::move(name))
{

}

MyTestActor::MyTestActor(const float timer, const int count)
    : mTimer(timer), mCount(count)
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

void serializeActor(YAML::Emitter &out, MyTestActor *myTestActor)
{
    out << YAML::Key << "Type"  << YAML::Value << "MyTestActor";
    out << YAML::Key << "Timer" << YAML::Value << myTestActor->mTimer;
    out << YAML::Key << "Count" << YAML::Value << myTestActor->mCount;
}

