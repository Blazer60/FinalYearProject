/**
 * @file HitInfo.h
 * @author Ryan Purse
 * @date 09/12/2023
 */


#pragma once

#include "Pch.h"

namespace engine
{
    class Component;
    class Actor;

    struct HitInfo
    {
        glm::vec3 normal;
        glm::vec3 position;
    };

    struct HitInfoExt
    {
        Actor *actorA;
        Actor *actorB;
        Component *componentA;
        Component *componentB;
        glm::vec3 hitNormalWorldB;
        glm::vec3 hitPositionWorldA;
        glm::vec3 hitPositionWorldB;
        HitInfo info;
    };
}
