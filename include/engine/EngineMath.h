/**
 * @file Math.h
 * @author Ryan Purse
 * @date 08/08/2023
 */


#pragma once

#include "Pch.h"

namespace math
{
    /**
     * \brief Deconstructs a transform into its inididual components.
     * \param transform The transform that you want to break down
     * \param position The position vector that will be set.
     * \param rotation The rotation that will be set.
     * \param scale The scale that will be set.
     */
    void decompose(const glm::mat4 &transform, glm::vec3 &position, glm::quat &rotation, glm::vec3 &scale);
}
