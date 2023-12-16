/**
 * @file CollisionInfo.h
 * @author Ryan Purse
 * @date 09/12/2023
 */


#pragma once

#include <Engine.h>

/**
 * @author Ryan Purse
 * @date 09/12/2023
 */
class CollisionInfo
    : public engine::Component
{
    void onBegin() override;
    void onCollisionBegin(engine::Actor* otherActor, Component* myComponent, Component* otherComponent, const engine::HitInfo& hitInfo) override;
    void onTriggerBegin(engine::Actor* otherActor, Component* myComponent, Component* otherComponent) override;
    void onDrawUi() override;

protected:
    Ref<engine::SoundComponent> mSound;

    SERIALIZABLE_COMPONENT(CollisionInfo);
};
