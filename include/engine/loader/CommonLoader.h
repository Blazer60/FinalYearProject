/**
 * @file CommonLoader.h
 * @author Ryan Purse
 * @date 23/02/2022
 */


#pragma once

#include <assimp/Importer.hpp>

namespace load
{
    glm::vec3 toVec3(const aiVector3D &v);
    glm::vec2 toVec2(const aiVector2D &v);
    glm::vec2 toVec2(const aiVector3D &v);
}

