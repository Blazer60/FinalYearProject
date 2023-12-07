/**
 * @file PhysicsConversions.h
 * @author Ryan Purse
 * @date 06/12/2023
 */


#pragma once


#include "Pch.h"

#include <LinearMath/btVector3.h>

namespace engine::physics
{
    inline btVector3 cast(const glm::vec3 &other)
    {
        return btVector3(other.x, other.y, other.z);
    }

    inline glm::vec3 cast(const btVector3 &other)
    {
        return glm::vec3(other.x(), other.y(), other.z());
    }

    inline btQuaternion cast(const glm::quat &other)
    {
        return btQuaternion(other.x, other.y, other.z, other.w);
    }

    inline glm::quat cast(const btQuaternion &other)
    {
        return glm::quat(other.w(), other.x(), other.y(), other.z());
    }
}
