/**
 * @file MyTestActor.h
 * @author Ryan Purse
 * @date 29/10/2023
 */


#pragma once

#include "Pch.h"
#include <Actor.h>


/**
 * @author Ryan Purse
 * @date 29/10/2023
 */
class MyTestActor
    : public engine::Actor
{
    SERIALIZABLE_ACTOR(MyTestActor);
public:
    explicit MyTestActor(std::string name);
    MyTestActor(float timer, int count);
    void onBegin() override;
    void onUpdate() override;
protected:
    float mTimer { 0 };
    int mCount { 0 };
};
