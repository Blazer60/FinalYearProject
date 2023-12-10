/**
 * @file physicsMesh.h
 * @author Ryan Purse
 * @date 10/12/2023
 */


#pragma once

#include <filesystem>
#include <BulletCollision/CollisionShapes/btTriangleIndexVertexArray.h>

#include "Pch.h"
#include "PhysicsMeshBuffer.h"

namespace load
{
    std::shared_ptr<engine::physics::MeshColliderBuffer> physicsMesh(const std::filesystem::path &path);
}
