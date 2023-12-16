/**
 * @file CollisionInfo.cpp
 * @author Ryan Purse
 * @date 09/12/2023
 */


#include "CollisionInfo.h"

#include <Engine.h>

void CollisionInfo::onBegin()
{
    mSound = mActor->getComponent<engine::SoundComponent>(false);
}

void CollisionInfo::onCollisionBegin(
 engine::Actor* otherActor, Component* myComponent, Component* otherComponent, const engine::HitInfo& hitInfo)
{
    MESSAGE("Collision Begin: %", hitInfo.normal);
    if (mSound.isValid())
        mSound->playSound();
}

void CollisionInfo::onTriggerBegin(engine::Actor* otherActor, Component* myComponent, Component* otherComponent)
{
    MESSAGE("Trigger Begin with: %", otherActor->getName());
    if (mSound.isValid())
        mSound->playSound();
}

void CollisionInfo::onDrawUi()
{
    engine::ui::PushID("Collision Info");
    if (engine::ui::TreeNodeEx("Collision Info Settings", ImGuiTreeNodeFlags_Framed | ImGuiTreeNodeFlags_SpanFullWidth | ImGuiTreeNodeFlags_DefaultOpen))
    {
        if (engine::ui::Button("Destroy Component"))
            mActor->removeComponent(this);

        engine::ui::TreePop();
    }

    engine::ui::PopID();
}

void serializeComponent(engine::serialize::Emitter &out, CollisionInfo *collisionInfo)
{
    out << engine::serialize::Key << "Component" << engine::serialize::Value << "CollisionInfo";
}

