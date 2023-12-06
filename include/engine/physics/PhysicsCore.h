/**
 * @file PhysicsCore.h
 * @author Ryan Purse
 * @date 06/12/2023
 */


#pragma once

#include "Pch.h"

#include <btBulletDynamicsCommon.h>

namespace engine
{
    /**
     * @author Ryan Purse
     * @date 06/12/2023
     */
    class PhysicsCore
    {
    public:
        PhysicsCore();
        ~PhysicsCore();

        std::unique_ptr<btDefaultCollisionConfiguration> configuration;
        std::unique_ptr<btCollisionDispatcher> dispatcher;
        std::unique_ptr<btBroadphaseInterface> overlappingPairCache;
        std::unique_ptr<btSequentialImpulseConstraintSolver> solver;
        std::unique_ptr<btDiscreteDynamicsWorld> dynamicsWorld;
    };
} // engine
