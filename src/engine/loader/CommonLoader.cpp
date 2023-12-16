/**
 * @file CommonLoader.cpp
 * @author Ryan Purse
 * @date 23/02/2022
 */


#include "CommonLoader.h"

namespace load
{
    glm::vec3 toVec3(const aiVector3D &v)
    {
        return { v.x, v.y, v.z };
    }

    glm::vec2 toVec2(const aiVector2D &v)
    {
        return { v.x, v.y };
    }

    glm::vec2 toVec2(const aiVector3D &v)
    {
        return { v.x, v.y };
    }
}


