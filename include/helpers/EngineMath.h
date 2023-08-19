/**
 * @file Math.h
 * @author Ryan Purse
 * @date 08/08/2023
 */


#pragma once

#include "Pch.h"

namespace math
{
    void decompose(const glm::mat4 &transform, glm::vec3 &position, glm::quat &rotation, glm::vec3 &scale);
}
