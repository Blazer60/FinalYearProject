/**
 * @file Rotator.h
 * @author Ryan Purse
 * @date 31/08/2023
 */


#pragma once

#include "Pch.h"
#include "Component.h"


/**
 * @author Ryan Purse
 * @date 31/08/2023
 */
class Rotator
    : public engine::Component
{
public:
    ~Rotator() override = default;

protected:
    void onUpdate() override;
    void onDrawUi() override;
    
protected:
    glm::vec3 mRotation { 0.f };
};
