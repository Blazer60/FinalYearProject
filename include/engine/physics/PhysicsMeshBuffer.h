/**
 * @file PhysicsMeshBuffer.h
 * @author Ryan Purse
 * @date 10/12/2023
 */


#pragma once

#include "Pch.h"

#include <vec3.hpp>
#include <btBulletCollisionCommon.h>

namespace engine::physics
{
    struct MeshDataBuffer
    {
        std::vector<glm::vec3> vertices;
        std::vector<int>       indices;
    };

    struct MeshColliderBuffer
    {
        std::vector<MeshDataBuffer> meshDataBuffers;
        std::vector<btIndexedMesh>  indexedMeshes;
        btTriangleIndexVertexArray  vertexArray;
    };
}
