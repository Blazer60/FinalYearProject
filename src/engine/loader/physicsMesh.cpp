/**
 * @file physicsMesh.cpp
 * @author Ryan Purse
 * @date 10/12/2023
 */


#include "physicsMesh.h"
#include "CommonLoader.h"
#include "EngineState.h"
#include "ResourcePool.h"

namespace load
{
    std::shared_ptr<engine::physics::MeshColliderBuffer> physicsMesh(const std::filesystem::path& path)
    {
        return engine::resourcePool->loadPhysicsMesh(path);
    }
}
