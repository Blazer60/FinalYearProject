/**
 * @file LookAtActor.h
 * @author Ryan Purse
 * @date 01/04/2024
 */


#pragma once

#include "Actor.h"
#include "Component.h"
#include "Pch.h"



/**
 * @author Ryan Purse
 * @date 01/04/2024
 */
class LookAtActor
    : public engine::Component
{
    SERIALIZABLE_COMPONENT(LookAtActor);
public:
    Ref<engine::Actor> trackedActor;
    engine::UUID trackedActorId = 0;
    bool invertZ = false;

    void onBegin() override;
    void onUpdate() override;
    void onDrawUi() override;
};
