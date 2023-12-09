/**
 * @file CollisionInfo.h
 * @author Ryan Purse
 * @date 09/12/2023
 */


#pragma once

#include "Component.h"
#include "Pch.h"

/**
 * @author Ryan Purse
 * @date 09/12/2023
 */
class CollisionInfo
    : public engine::Component
{
    void onCollisionBegin(engine::Actor* otherActor, Component* myComponent, Component* otherComponent, const engine::HitInfo& hitInfo) override;
    void onDrawUi() override;

    SERIALIZABLE_COMPONENT(CollisionInfo);
};
