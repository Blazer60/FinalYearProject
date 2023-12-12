/**
 * @file PhysicsCore.h
 * @author Ryan Purse
 * @date 06/12/2023
 */


#pragma once

#include "Pch.h"

#include <btBulletDynamicsCommon.h>
#include <unordered_set>

#include "Component.h"
#include "HitInfo.h"
#include "Mesh.h"
#include "PhysicsDebugDrawer.h"

namespace engine
{
    void physicsNearCallback(btBroadphasePair &collisionPair, btCollisionDispatcher &dispatcher, const btDispatcherInfo &dispatcherInfo);

    /**
     * @author Ryan Purse
     * @date 06/12/2023
     */
    class PhysicsCore
    {
        friend void physicsNearCallback(btBroadphasePair &collisionPair, btCollisionDispatcher &dispatcher, const btDispatcherInfo &dispatcherInfo);
    public:
        PhysicsCore();
        ~PhysicsCore();

        void clearContainers();
        void renderDebugShapes() const;
        void resolveCollisoinCallbacks();

        std::unique_ptr<btDefaultCollisionConfiguration> configuration;
        std::unique_ptr<btCollisionDispatcher> dispatcher;
        std::unique_ptr<btBroadphaseInterface> overlappingPairCache;
        std::unique_ptr<btSequentialImpulseConstraintSolver> solver;
        std::unique_ptr<btDiscreteDynamicsWorld> dynamicsWorld;
        std::unique_ptr<PhysicsDebugDrawer> debugDrawer;

    protected:
        void createHitInfo(const btManifoldArray &manifoldArray, Component *componentA, Component *componentB);
        std::unordered_map<std::string, HitInfoExt> mCollisions;
        std::unordered_set<std::string> mCurrentCollisions;

        SharedMesh mDefaultCube;
        SharedMesh mDefaultSphere;
    };
} // engine
