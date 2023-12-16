/**
 * @file Rotator.h
 * @author Ryan Purse
 * @date 31/08/2023
 */


#pragma once

#include "Pch.h"
#include "Component.h"
#include "Serializer.h"


/**
 * @author Ryan Purse
 * @date 31/08/2023
 */
class Rotator
    : public engine::Component
{
    SERIALIZABLE_COMPONENT(Rotator);
public:
    Rotator() = default;
    explicit Rotator(const glm::vec3 &rotation);
    ~Rotator() override = default;

protected:
    void onUpdate() override;
    void onDrawUi() override;
    
protected:
    glm::vec3 mRotation { 0.f };
};
